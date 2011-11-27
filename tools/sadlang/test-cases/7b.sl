// blink blue led; faster at the start
when System.time < 2s:
    use BlueLed, period 100ms;
else:
    use BlueLed, period 2000ms;
end
