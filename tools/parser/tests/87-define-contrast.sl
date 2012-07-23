// constrasting and blurring sensor inputs

define Accel1 AnalogIn, channel 1;
define Accel2 AnalogIn, channel 2;

define Blurred smoothen(Accel1);
define Contrasted sharpen(Accel2, 5, 5);

read Blurred; read Contrasted;
