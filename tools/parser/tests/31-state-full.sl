set LARGE_RANDOM_NUMBER_ENCOUNTERED false;
when Random > 50000:
    set LARGE_RANDOM_NUMBER_ENCOUNTERED true;
end;

read LARGE_RANDOM_NUMBER_ENCOUNTERED;
