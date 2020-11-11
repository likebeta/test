## 启动

```
docker-compose -f docker-compose-net1.yml up -d
docker-compose -f docker-compose-net2.yml up -d
```

## 说明

`net2-redis-1` 和 `net2-redis-3` 都有两个网卡，启动和关闭时注意依赖顺序，attach每个container后`ifconfig`看下更容易理解。

## 疑问

`net2-redis-1` 使用 `ping redis1` 的时候会轮询net1和net2的redis1服务，使用的都是net1下的ip； 使用 `ping -I eth1 redis1`不同，目前不知道怎么指定net2下的ip去ping。
