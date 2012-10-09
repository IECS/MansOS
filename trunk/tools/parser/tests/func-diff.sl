
define AverageDifference average(take(difference(TotalSolarRadiation, PhotosyntheticRadiation), 10));

read AverageDifference, period 500ms;

output Serial;
