extern "C" {
    int code(int seed, char *data, int datalen, char *out)
    {
        unsigned int randint = seed;
        char randchar;
        for(int i=0;i<datalen;i++)
        {
            randint = randint * 1103515245 + 12345;
            randint = (randint / 65536) % 32768;
            randchar = randint % 255;
            out[i]=data[i]^randchar;
        }
        return 0;
    }
}
