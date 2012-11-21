define MyInput DigitalIn, port 2, pin 0;
read MyInput;
when invertFilter(filterEqual(MyInput, 1)):
   use Print, format "is zero!\n";
else:
   use Print, format "is one!\n";
end
