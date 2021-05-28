#pragma once

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include <cassert>
#include <memory>
#include <algorithm>
#include <initializer_list>
#include <utility>

#define RANDOMENGINE std::default_random_engine
#define CSV ","

#define OUTEREDGERADIUS 90.0
#define INNEREDGERADIUS 20.0
#define SIDEDISTANCE 1.0

#define CHANNEL_DISTANCE 4.5
#define TARGET_RADIUS 0.9
#define WORLD_SIZE 100.0

#define DRIVER_SIZE 14.0
#define RESISTOR_SIZE 1.2

#define LED3_SIZE 3.2
#define LED3_RANGE 64.0
#define LED3_POWER 1.0

#define LED5_SIZE 5.5
#define LED5_RANGE 40.0
#define LED5_POWER 4.0

#define LED8_SIZE 9.0
#define LED8_RANGE 50.0
#define LED8_POWER 10.0

#define LEDX_SIZE 11.0
#define LEDX_RANGE 120.0
#define LEDX_POWER 40.0

#define LEDB_SIZE 20.5
#define LEDB_RANGE WORLD_SIZE
#define LEDB_POWER 500.0
