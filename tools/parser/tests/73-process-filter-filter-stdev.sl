read Light, period 1s;

process Light lightFiltredFromDown, filter >100;

process lightFiltredFromDown lightFiltred, filter <200;

process lightFiltred filtredLightStdev, stdev 10;