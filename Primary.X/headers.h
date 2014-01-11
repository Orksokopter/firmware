#include <p18f4620.h>
#include <timers.h>
#include <capture.h>
#include <adc.h>
#include "..\Common\headers.h"
#include "core.h"
#include "adc.h"
#include "communications.h"
#include "controller.h"
#include "motors.h"
#include "rc.h"
#include "accel.h"

// Fake
// #define Delay50Cycles() Nop();Nop();
// Das hier hat offenbar für die Motorcontroller gereicht, dort also noch ändern!

