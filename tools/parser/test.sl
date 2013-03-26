// read Light;
// when Light < 100:
//    read Humidity, period 2000;
//    when 1 > 2:
//        read Temperature;
//    end
// end

// use RedLed, once;
// use GreenLed, times 2;
// use Print, once;

when Led:
  read Light;
end
