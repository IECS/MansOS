read Humidity;

when Humidity.value > 100:
     // Note: "Humidity" is synonym to "Humidity.value"!
     use Print, once, format "Humidity > 100, value=%d\n", arg1 Humidity;
end

when Humidity.isError:
     use Print, once, format "Humidity sensor failed!\n";
end
