#include "headers.h"
#include "controller.h"

struct motor_data_t motor_data = {0, 0, 0, 0};

byte debug1 = 42;
byte debug2 = 42;
byte debug3 = 42;
byte debug4 = 42;

struct controllerstate_t {
	int32_t integral_pitch;
	int32_t integral_roll;
	int32_t integral_yaw;
} controllerstate = {0, 0, 0};

struct gyro_offsets_t gyro_offsets = {0, 0, 0};

int16_t RollSum, PitchSum, YawSum;
int16_t LastRollE, LastPitchE, LastYawE;
int16_t LastControlPitch, LastControlRoll, LastControlYaw;

void do_control(void)
{
	int32_t motors_pitch;
	int32_t motors_roll;
	int32_t motors_yaw;

	int32_t difference;

	uint8_t throttle;

	// Diese Abfrage hier ist wichtig, weil sie das Regelintervall bestimmt:
	if (!new_gyro_values_available)
		return;
	new_gyro_values_available = false;

	acquire_accel_values();

	// Wertebereich adc_values: 0 .. 512 .. 1024
	motors_pitch = ((int32_t)adc_values.pitch - gyro_offsets.pitch_zero);
	motors_roll = -((int32_t)adc_values.roll - gyro_offsets.roll_zero);
	motors_yaw = -((int32_t)adc_values.yaw - gyro_offsets.yaw_zero);

	// Macht UAVX auch so:
	motors_pitch /= 2;
	motors_roll /= 2;
	motors_yaw /= 2;

	if (rc_data.pb8)
	{
		controllerstate.integral_pitch = controllerstate.integral_roll = controllerstate.integral_yaw = 0;
	}
	else
	{
		/*
		controllerstate.integral_pitch += motors_pitch / 8;
		controllerstate.integral_roll += motors_roll / 8;
		controllerstate.integral_yaw += motors_yaw / 8;
		*/

		controllerstate.integral_pitch += motors_pitch;
		controllerstate.integral_roll += motors_roll;
		controllerstate.integral_yaw += motors_yaw;

		controllerstate.integral_pitch = Limit(controllerstate.integral_pitch, -256 * parameters[PARAM_PITCH_ILIMIT], 256 * parameters[PARAM_PITCH_ILIMIT]);
		controllerstate.integral_roll = Limit(controllerstate.integral_roll, -256 * parameters[PARAM_ROLL_ILIMIT], 256 * parameters[PARAM_ROLL_ILIMIT]);
		controllerstate.integral_yaw = Limit(controllerstate.integral_yaw, -256 * parameters[PARAM_YAW_ILIMIT], 256 * parameters[PARAM_YAW_ILIMIT]);
	}

	throttle = Limit((rc_data.left_vertical - 200) / 8, 0, 255);

	// parameters[PARAM_PITCH_KP] = parameters[PARAM_ROLL_KP] = parameters[PARAM_YAW_KP] = Limit(rc_data.ctrl7 / 2, 0, 255);
	// parameters[PARAM_PITCH_KI] = parameters[PARAM_ROLL_KI] = parameters[PARAM_YAW_KI] = Limit(rc_data.ctrl5, 0, 255);

	parameters[PARAM_MISC_ACC_HORIZ_KI] = parameters[PARAM_MISC_ACC_VERT_KI] = Limit(rc_data.ctrl5, 0, 255);

	if (rc_data.sw2)
	{
		motor_data.front =
		motor_data.left =
		motor_data.right =
		motor_data.rear = throttle;

		// Jeweils:
		// - Geschwindigkeitsänderung der Motoren berechnen
		// - Änderung limitieren auf das kleinste was möglich ist, ohne dass ein Motor unter 0 oder über 255 geht
		// - Änderung den Motoren zuweisen

		difference = (motors_pitch * parameters[PARAM_PITCH_KP]) / 512 +
		             (controllerstate.integral_pitch * parameters[PARAM_PITCH_KI]) / 255 +
		             ((accel_values.z - accel_offsets.z) * parameters[PARAM_MISC_ACC_HORIZ_KI]) / 4096 +
		             (rc_data.right_vertical / 16);
		difference = Limit(difference, ((int16_t)motor_data.front-255), ((int16_t)motor_data.front));
		difference = Limit(difference, -((int16_t)motor_data.rear), (255-(int16_t)motor_data.rear));
		motor_data.front -= difference;
		motor_data.rear += difference;

		difference = (motors_roll * parameters[PARAM_ROLL_KP]) / 512 +
		             (controllerstate.integral_roll * parameters[PARAM_ROLL_KI]) / 255 +
		             ((accel_values.y - accel_offsets.y) * parameters[PARAM_MISC_ACC_VERT_KI]) / 4096 +
		             (rc_data.right_horizontal / 16);
		difference = Limit(difference, ((int16_t)motor_data.left-255), ((int16_t)motor_data.left));
		difference = Limit(difference, -((int16_t)motor_data.right), (255-(int16_t)motor_data.right));
		motor_data.left -= difference;
		motor_data.right += difference;

		difference = (motors_yaw * parameters[PARAM_YAW_KP]) / 512 + (controllerstate.integral_yaw * parameters[PARAM_YAW_KI]) / 255 + (rc_data.left_horizontal / 16);
		difference = Limit(difference, ((int16_t)motor_data.left-255), ((int16_t)motor_data.left));
		difference = Limit(difference, ((int16_t)motor_data.right-255), ((int16_t)motor_data.right));
		difference = Limit(difference, -((int16_t)motor_data.front), (255-(int16_t)motor_data.front));
		difference = Limit(difference, -((int16_t)motor_data.rear), (255-(int16_t)motor_data.rear));
		motor_data.left -= difference;
		motor_data.right -= difference;
		motor_data.front += difference;
		motor_data.rear += difference;

		motor_data.front = Limit(motor_data.front, parameters[PARAM_MISC_IDLE_SPEED], 255);
		motor_data.left = Limit(motor_data.left, parameters[PARAM_MISC_IDLE_SPEED], 255);
		motor_data.right = Limit(motor_data.right, parameters[PARAM_MISC_IDLE_SPEED], 255);
		motor_data.rear = Limit(motor_data.rear, parameters[PARAM_MISC_IDLE_SPEED], 255);
	}
	else
	{
		motor_data.front = motor_data.left = motor_data.right = motor_data.rear = 0;
	}
}

void do_control2(void)
{
	int16_t RollRateADC, PitchRateADC, YawRateADC;
	int16_t RollRate, PitchRate, YawRate;
	int16_t DesiredRoll, DesiredPitch, DesiredYaw, DesiredThrottle;
	int16_t ControlRoll, ControlPitch;
	int16_t RollE, PitchE, YawE;
	int16_t RollL, PitchL, YawL;
	int16_t Temp;
	int16_t CurrThrottle;
	int16_t PWMLeft, PWMRight, PWMFront, PWMBack;
	int24_t MaxMotor;
	int24_t DemandSwing;
	int24_t ScaleHigh, ScaleLow, Scale;

	int16_t RollKp = -20;
	int16_t RollKi = -12;
	int16_t RollKd = 50;
	int16_t RollIntLimit = 3;
	int16_t PitchKp = -20;
	int16_t PitchKi = -12;
	int16_t PitchKd = 50;
	int16_t PitchIntLimit = 3;
	int16_t YawKp = -30;
	int16_t YawKi = -25;
	int16_t YawKd = 0;
	int16_t YawLimit = 25;
	int16_t YawIntLimit = 2;
	int16_t StickFeedForward = 24;

	// Diese Abfrage hier ist wichtig, weil sie das Regelintervall bestimmt:
	if (!new_gyro_values_available)
		return;
	new_gyro_values_available = false;

	RollRateADC = adc_values.roll;
	PitchRateADC = adc_values.pitch;
	YawRateADC = adc_values.yaw;

	PitchRate = -(PitchRateADC - (int16_t)gyro_offsets.pitch_zero);
	RollRate = RollRateADC - (int16_t)gyro_offsets.roll_zero;
	YawRate = YawRateADC - (int16_t)gyro_offsets.yaw_zero;

	PitchRate = PitchRate / 2;
	RollRate = RollRate / 2;
	YawRate = YawRate / 2;

	// Wie die vorverarbeitet werden, habe ich mir nicht so genau angeguckt... reicht ja erstmal, wenn er schwebt.
	DesiredPitch = -rc_data.right_vertical / 8;
	DesiredRoll = -rc_data.right_horizontal / 8;
	DesiredYaw = -rc_data.left_horizontal / 8;
	DesiredThrottle = rc_data.left_vertical / 8;

	ControlPitch = DesiredPitch;
	ControlRoll = DesiredRoll;

	PitchE = PitchRate / 2;
	RollE = RollRate / 2;
	YawE = YawRate;

	PitchSum += PitchRate;
	RollSum += RollRate;
	YawSum += YawRate;

	PitchSum = Limit(PitchSum, -256*PitchIntLimit, 256*PitchIntLimit);
	RollSum = Limit(RollSum, -256*RollIntLimit, 256*RollIntLimit);
	YawSum = Limit(YawSum, -256*YawIntLimit, 256*YawIntLimit);

	PitchSum = Decay1(PitchSum);
	RollSum = Decay1(RollSum);
	YawSum = Decay1(YawSum);
	YawSum = Decay1(YawSum);

	YawE += DesiredYaw;

	PitchL = PitchE * PitchKp + (LastPitchE - PitchE) * PitchKd;
	RollL = RollE * RollKp + (LastRollE - RollE) * RollKd;
	YawL = YawE * YawKp + (LastYawE - YawE) * YawKd;

	LastPitchE = PitchE;
	LastRollE = RollE;
	LastYawE = YawE;

	PitchL = PitchL / 16;
	RollL = RollL / 16;
	YawL = YawL / 16;

	PitchL += PitchSum * PitchKi / 512;
	RollL += RollSum * RollKi / 512;
	YawL += YawSum * YawKi / 256;
	
	YawL = Limit(YawL, -YawLimit, YawLimit);

	PitchL -= ( (ControlPitch - LastControlPitch) * StickFeedForward / 16 );
	RollL -= ( (ControlRoll - LastControlRoll) * StickFeedForward / 16 );

	LastControlPitch = ControlPitch;
	LastControlRoll = ControlRoll;

	PitchL -= ControlPitch;
	RollL -= ControlRoll;

	CurrThrottle = DesiredThrottle;

	Temp = (255 * 90 + 50) / 100;
	CurrThrottle = Limit(CurrThrottle, 0, Temp);

	PWMLeft = PWMRight = PWMFront = PWMBack = CurrThrottle;

	PWMLeft += -RollL - YawL;
	PWMRight += RollL - YawL;
	PWMFront += -PitchL + YawL;
	PWMBack += PitchL + YawL;

	MaxMotor = Max(PWMLeft, PWMRight);
	MaxMotor = Max(MaxMotor, PWMFront);
	MaxMotor = Max(MaxMotor, PWMBack);

	DemandSwing = MaxMotor - CurrThrottle;

	if (DemandSwing > 0)
	{
		ScaleHigh = (255 - (int24_t)CurrThrottle) * 256 / DemandSwing;
		ScaleLow = ((int24_t)CurrThrottle - 3) * 256 / DemandSwing;
		Scale = Min(ScaleHigh, ScaleLow); // "just in case"
		Scale = Max(1, Scale);

		if (Scale < 256)
		{
			PitchL = PitchL * Scale / 256;
			RollL = RollL * Scale / 256;
			YawL = YawL * Scale / 256;

			PWMLeft = PWMRight = PWMFront = PWMBack = CurrThrottle;

			PWMLeft += -RollL - YawL;
			PWMRight += RollL - YawL;
			PWMFront += -PitchL + YawL;
			PWMBack += PitchL + YawL;
		}
	}

	if (rc_data.sw2)
	{
		motor_data.front = Max(PWMFront, 3);
		motor_data.left = Max(PWMLeft, 3);
		motor_data.right = Max(PWMRight, 3);
		motor_data.rear = Max(PWMBack, 3);
	}
	else
	{
		motor_data.front = motor_data.left = motor_data.right = motor_data.rear = 0;
	}

	debug1 = PWMFront;
	debug2 = PWMLeft;
	debug3 = PWMRight;
	debug4 = PWMBack;

	motors_need_updating = true;
}