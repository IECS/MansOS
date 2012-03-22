//use RedLed, period 2000;

//when 1 != 2:
//    use Foobar1, param1 1000, param2 1s, param3 1500ms;
//elsewhen 2 < 4:
//    use Foobar2;
//else:
//    use Foobar3;
//end;


use RedLed, period 1000ms;
when 1 < 2:
  read Humidity, period 2000ms;
  when 2 < 3 and 3 < 4:
    use RedLed, period 3000ms;
    read Light, period 2000ms;
  end
end
