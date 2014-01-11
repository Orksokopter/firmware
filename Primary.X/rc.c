#include "headers.h"
#include "rc.h"

void decode_rc_signals(void)
{
	if (!new_rc_values_available)
		return;
	new_rc_values_available = false;

	rc_data.left_vertical = raw_rc_data[1] - RC_SHORT_PULSE_LENGTH;

	rc_data.left_horizontal = raw_rc_data[2] - RC_MIDDLE_PULSE_LENGTH;
	rc_data.right_vertical = raw_rc_data[3] - RC_MIDDLE_PULSE_LENGTH;
	rc_data.right_horizontal = raw_rc_data[4] - RC_MIDDLE_PULSE_LENGTH;

	// Durch wildes Herumprobieren mit den Mischereinstellungen sind die folgenden
	// Positionen (mit 10 Prozentpunkten Abstand) herausgekommen:

	// SW2 PB8 SW1  Pos
	//   0   0   0   80
	//               70
	//   0   0   1   60
	//               50
	//               30
	//   1   0   0   20
	//               10
	//   1   0   1    0
	//              -10
	//   0   1   0  -20
	//              -30
	//   0   1   1  -40
	//              -50
	//              -70
	//   1   1   0  -80
	//              -90
	//   1   1   1 -100

	if (raw_rc_data[6] < -10L * RC_PULSE_LENGTH_RANGE / 200 + RC_MIDDLE_PULSE_LENGTH)
		rc_data.pb8 = true;
	else
		rc_data.pb8 = false;

	if (
		raw_rc_data[6] < 30L * RC_PULSE_LENGTH_RANGE / 200 + RC_MIDDLE_PULSE_LENGTH  && raw_rc_data[6] > -10L * RC_PULSE_LENGTH_RANGE / 200 + RC_MIDDLE_PULSE_LENGTH ||
		raw_rc_data[6] < -70L * RC_PULSE_LENGTH_RANGE / 200 + RC_MIDDLE_PULSE_LENGTH
	)
		rc_data.sw2 = true;
	else
		rc_data.sw2 = false;

	if (
		raw_rc_data[6] < 70L * RC_PULSE_LENGTH_RANGE / 200 + RC_MIDDLE_PULSE_LENGTH && raw_rc_data[6] > 50L * RC_PULSE_LENGTH_RANGE / 200 + RC_MIDDLE_PULSE_LENGTH ||
		raw_rc_data[6] < 10L * RC_PULSE_LENGTH_RANGE / 200 + RC_MIDDLE_PULSE_LENGTH && raw_rc_data[6] > -10L * RC_PULSE_LENGTH_RANGE / 200 + RC_MIDDLE_PULSE_LENGTH ||
		raw_rc_data[6] < -30L * RC_PULSE_LENGTH_RANGE / 200 + RC_MIDDLE_PULSE_LENGTH && raw_rc_data[6] > -50L * RC_PULSE_LENGTH_RANGE / 200 + RC_MIDDLE_PULSE_LENGTH ||
		raw_rc_data[6] < -90L * RC_PULSE_LENGTH_RANGE / 200 + RC_MIDDLE_PULSE_LENGTH
	)
		rc_data.sw1 = true;
	else
		rc_data.sw1 = false;

	rc_data.ctrl5 = raw_rc_data[5] - RC_MIDDLE_PULSE_LENGTH;

	rc_data.ctrl7 = raw_rc_data[7] - RC_SHORT_PULSE_LENGTH;
}