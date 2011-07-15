#!/usr/bin/python


import sys
import time
from socket import *


# user: normal processes executing in user mode
# nice: niced processes executing in user mode
# system: processes executing in kernel mode
# idle: twiddling thumbs
# iowait: waiting for I/O to complete
# irq: servicing interrupts
# softirq: servicing softirq

def get_cpu_load():
    fn = "/proc/stat"
    f = open(fn, "r")
    l = f.readline()
    f.close()

    load_a = l.split()
    user = int(load_a[1])
    nice = int(load_a[2])
    sys = int(load_a[3])
    idle = int(load_a[4])
    iowait = int(load_a[5])

    total = user + nice + sys + idle + iowait
    load = user + sys

    return (load,total)


def main():
    serverHost = '172.31.0.255'
    serverPort = 2348

    sockobj = socket(AF_INET, SOCK_DGRAM)
    sockobj.setsockopt(SOL_SOCKET, SO_BROADCAST, 1)
    sockobj.bind(("",0))
    sockobj.connect((serverHost, serverPort))

    (old_load,old_total) = get_cpu_load()
    while 1:
        time.sleep(1)

        (new_load,new_total) = get_cpu_load()

        total = new_total - old_total
        load = new_load - old_load

        p = float(load) / float(total)

        x = int(p * 10)
        r = (p*10) - x
        if r < .5:
            x = x - 1

        str = "0"
        for i in range(0 , int(x)):
            str = str + "F"

        u = "uid 0FFFFFFFFF %s\n" % (str)
        c = "cab 0FFFFFFFFF %s\n" % (str)
        print u
        print c
        sockobj.send(u)
        sockobj.send(c)

    sockobj.close() 

if __name__ == "__main__":
    main()

