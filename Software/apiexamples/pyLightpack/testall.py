import lightpack, time
lpack = lightpack.lightpack('127.0.0.1', 3636, [2,3,6,7,1,1,1,4,5,1] )
lpack.connect()

print("Lock: %s" % lpack.lock())
print("turnOn: %s" % lpack.turnOn());

num = int(lpack.getCountLeds())
print("Num leds: %s" % num)

print("Status: %s" % lpack.getStatus())
print("Profile: %s" % lpack.getProfile())
print("Profiles: %s" % lpack.getProfiles())
print("getAPIStatus: %s" % lpack.getAPIStatus())

for i in range(0, num-1):
    print("setColor%d: %s" % (i, lpack.setColor(i, 255, 0, 0)))
    time.sleep(0.1)
time.sleep(1)

print("setColorToAll: %s" % lpack.setColorToAll(0, 0, 0))
time.sleep(1)

print("turnOff: %s" % lpack.turnOff());

lpack.disconnect()
