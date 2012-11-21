read Random;
when min(Random) < 10000:
    use print, format "rand number < 10000 encountered!\n", once;
else:
    use print, format "meh!\n", once;
end;
