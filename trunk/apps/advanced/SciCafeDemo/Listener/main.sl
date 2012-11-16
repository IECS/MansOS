NetworkRead RemotePacket(Light);
when RemotePacket.Light < 50:
    use RedLed, on;
else:
    use RedLed, off;
end
