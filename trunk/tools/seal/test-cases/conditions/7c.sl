when System.time < 2s:
    use BlueLed, period 100ms;
    use RedLed, period 200ms;
elsewhen System.time < 6s:
    use BlueLed, period 500ms;
    use RedLed, period 1000ms;
else:
    use BlueLed, period 2000ms;
    use RedLed, period 3000ms;
end
