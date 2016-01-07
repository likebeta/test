#!/usr/bin/env python
# -*- coding=utf-8 -*-

# Author: likebeta <ixxoo.me@gmail.com>
# Create: 2015-12-31

import json
import datetime
import calendar


def log(*args):
    prefix = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    msg = [str(arg) for arg in args]
    print '%s | %s' % (prefix, ' '.join(msg))


class CheckTool(object):
    def __init__(self, year, month):
        self.month_days = None
        self.staff_info = {}
        self.staff_record = {}
        self.staff_stat = {}
        self.year = year
        self.month = month
        self.start_weekday = self.weekday(year, month, 1)
        self.days = calendar.monthrange(year, month)[1]
        self.immunity_count = 2
        self.total_minute = int(3600*9.5)

    def load_staff_info(self, path):
        log('loading staff info from', path)
        data = json.load(open(path))
        for item in data['staff']:
            self.staff_info[item['id']] = item
        log('loading staff info done')

    def load_attendance_record(self, path):
        log('loading attendance record from', path)
        with open(path) as f:
            lines = f.readlines()
            descs = lines[::2]
            records = lines[1::2]
            if len(descs) != len(records):
                raise Exception('desc not equal record')
            for desc, record in zip(descs, records):
                self.__parse_record(desc, record)
        log('loading attendance record done')

    def __compare(self, left, right):
        left = int(left.replace(':', ''))
        right = int(right.replace(':', ''))
        if left > right:
            return 1
        elif left == right:
            return 0
        else:
            return -1

    def __diff_time(self, start, end):
        start = start.split(':')
        end = end.split(':')
        start_delta = datetime.timedelta(minutes=int(start[1]), hours=int(start[0]))
        end_delta = datetime.timedelta(minutes=int(end[1]), hours=int(end[0]))
        return (end_delta - start_delta).seconds

    def __parse_record(self, desc, record):
        ds = desc.rstrip().split('\t')
        number = int(ds[2])
        rs = record.rstrip().split('\t')
        for i, record in enumerate(rs):
            if not record:
                rs[i] = None
            elif len(record) == 5:
                start, end = None, None
                if self.__compare(record, '12:00') >= 0:
                    end = record
                else:
                    start = record
                rs[i] = [start, end]
            else:
                start, end = None, None
                tmp = record[:5]
                if self.__compare(tmp, '12:00') < 0:
                    start = tmp
                tmp = record[-5:]
                if self.__compare(tmp, '12:00') >= 0:
                    end = tmp
                rs[i] = [start, end]

        if len(rs) < self.days:
            rs.extend([None] * (self.days - len(rs)))
        self.staff_record[number] = rs

    def parse(self):
        """
        1. 正常上班时间是9:00-18:30或者9:30-19:00，（弹性时间：例如：9:10到，则18:40下班）
        2. 9:30-10:00有两次机会不算迟到，超过两次机会则每次扣50，若超过10:00上班则为迟到
        3. 加班到20:30(1级加班)，第二天到岗为10:00前，加班到21:30(2级加班)，第二天到岗为10:30前
        4. 只要提前下班就视为早退
        """
        for number, records in self.staff_record.iteritems():
            overtime = False
            bitmap = self.staff_info[number]['bitmap']
            immunity_count = self.immunity_count
            stat_info = {'late': [], 'absenteeism': [], 'forget': [], 'early': [], 'overtime': []}
            for i, record in enumerate(records):
                _ot = overtime
                overtime = False
                weekday = (self.start_weekday + i) % 7
                if bitmap[weekday] == '0':  # 不上班
                    continue

                # 如果是双休, 星期六不计算
                if weekday == 5 and self.double_cease(self.year, self.month, i + 1):
                    continue

                if self.__check_absenteeism(record):
                    stat_info['absenteeism'].append(i+1)
                    continue

                if self.__check_forget(record):
                    stat_info['forget'].append(i+1)

                # 是否早上迟到
                am_valid_time = '9:00'
                if _ot == 2:    # 2级加班
                    am_late_time = '10:30'
                    am_die_time = '10:30'
                elif _ot == 1:  # 1级加班
                    am_late_time = '10:00'
                    am_die_time = '10:00'
                else:
                    am_late_time = '09:30'
                    am_die_time = '10:00'

                if record[0]:   # 早上打卡
                    late_level = self.__check_am_late(record, am_late_time, am_die_time)
                    if late_level == 1 and immunity_count > 0:
                        immunity_count -= 1
                    elif late_level > 0:
                        stat_info['late'].append(i+1)
                if record[1]:   # 晚上打卡
                    pm_early_time = self.__get_pm_early_time(record, am_valid_time, am_late_time)
                    if self.__check_pm_early(record, pm_early_time):
                        stat_info['early'].append(i+1)

                    _overtime = self.__check_overtime(record)
                    if _overtime > 0:
                        overtime = _overtime
                        stat_info['overtime'].append(i + 1)
            self.staff_stat[number] = stat_info

    def __check_absenteeism(self, record):
        """
        检查是否旷工(全天无记录)
        """
        return record is None

    def __check_forget(self, record):
        """
        检查是否忘记打卡(上午或者下午无记录)
        """
        return None in record

    def __check_am_late(self, record, am_late_time, am_die_time):
        """
        检查是否迟到, 1为可抵消, 2为不可抵消
        """
        if self.__compare(record[0], am_die_time) > 0:
            return 2
        if self.__compare(record[0], am_late_time) > 0:
            return 1
        return 0

    def __check_pm_early(self, record, pm_early_time):
        """
        检查是否早退
        """
        return self.__compare(record[1], pm_early_time) < 0

    def __get_pm_early_time(self, record, am_valid_time, am_late_time):
        if record[0] is None:
            return '18:30'
        elif self.__compare(record[0], am_late_time) > 0:
            return '19:00'
        if self.__compare(record[0], am_valid_time) < 0:
            return '18:30'
        else:
            start = record[0].split(':')
            start_tm = datetime.datetime(2015, 1, 1, int(start[0]), int(start[1]))
            end_tm = start_tm + datetime.timedelta(seconds=self.total_minute)
            return end_tm.strftime('%H:%M')

    def __check_overtime(self, record):
        if self.__compare(record[1], '21:30') > 0:
            return 2
        if self.__compare(record[1], '20:30') > 0:
            return 1
        return 0

    @classmethod
    def weekday(cls, year, month, day):
        d = datetime.datetime(year, month, day)
        return d.weekday()

    @classmethod
    def double_cease(cls, year, month, day):
        """
        2015-12-28 这一周是单休
        """
        d1 = datetime.datetime(2015, 12, 28)
        d2 = datetime.datetime(year, month, day)
        if d2 < d1:
            raise Exception('after 2015-12-28 please')
        diff_days = (d2 - d1).days
        diff_week = diff_days / 7
        return diff_week % 2

    def __stringify(self, l):
        l = [str(i) for i in l]
        s = ','.join(l)
        if s:
            return '[' + s + ']'
        return ''

    def save(self, path):
        s = '姓名\t加班\t迟到\t早退\t旷工\t未打卡'
        for number, info in self.staff_stat.iteritems():
            name = self.staff_info[number]['name'].encode('utf-8')
            overtime = self.__stringify(info['overtime'])
            late = self.__stringify(info['late'])
            forget = self.__stringify(info['forget'])
            early = self.__stringify(info['early'])
            absenteeism = self.__stringify(info['absenteeism'])
            s += '\n{name}\t{overtime}\t{late}\t{early}\t{absenteeism}\t{forget}'.format(**locals())

        with open(path, 'w') as f:
            f.write(s)
        log('save to', path)


if __name__ == '__main__':
    import os
    import sys
    import time

    staff_path = os.path.abspath(sys.argv[1])
    record_path = os.path.abspath(sys.argv[2])
    file_name = os.path.basename(record_path)
    save_path = os.path.dirname(record_path) + '/stat' + file_name
    tm = time.strptime(file_name, '%Y%m.txt')
    ct = CheckTool(tm.tm_year, tm.tm_mon)
    ct.load_staff_info(staff_path)
    ct.load_attendance_record(record_path)
    ct.parse()
    ct.save(save_path)
