# personCounter

## Prerequisites

This example is configured for ESP32 MCU of 4MB Flash.
Install the following packages for ESP_IDF (for Ubuntu and Debian):

```shell
apt-get install -y wget
apt-get install -y flex
apt-get install -y bison
apt-get install -y gperf
apt-get install -y python3
apt-get install -y python3-pip
apt-get install -y python3-setuptools
apt-get install -y cmake
apt-get install -y ninja-build
apt-get install -y ccache
apt-get install -y libffi-dev
apt-get install -y libssl-dev
apt-get install -y dfu-utils
apt-get install -y libusb-1.0-0
```

## Installation

After you've completed the hardware settings:

* Clone and enter the repository.

* Upload all project components `git submodule update --init --recursive`.
The project is using [IDF v4.2.2](https://github.com/espressif/esp-idf/tree/v4.2.2).

* Install the idf tool:

```shell
./esp-idf/install.sh
source esp-idf/export.sh
```

- Install [mosquitto](https://mosquitto.org/download/) MQTT server.

- Allow mosquitto to accept remote connexions, by adding the following to the configuration file:

```
allow_anonymous true
listener 1883 <ip_address_of_the_server>
```

## Configuration

The default configuration of the project is:
* DEVICE_ID = "Default"
* DEVICE_KEY = "default"
* BROKER_PORT = 1883

It can be accessed by typing `idf.py menuconfig` and going under personCounter app setting.

The following parameters are required, and need to be set:
* WiFi SSID and password for internet connexion.
* The MQTT server ip address (same as configured in mosquito conf).

## Build, Compile and Monitor

* **Connect** ESP32 module to the computer.

* **Run** mosquitto server `mosquitto -c <path_to_config_file>`.

* **Configure** your connexion parameters `idf.py menuconfig`.

* **Subscribe** to mosquitto topic `mosquitto_sub -L 'mqtt://DEVICE_ID:DEVICE_KEY@<ip_address_of_the_server>:BROKER_PORT/iot/dev/DEVICE_ID/data' -v`.

* **Build, Flash and Monitor** the application using `idf.py` (`idf.py --help`).

## Output

The output of the MQTT subscriber looks like the following.
For the moment, the ESP generate dummy counter data that increment after publishing.

```
iot/dev/Default/data { "type": "count", "value": 0 }
iot/dev/Default/data { "type": "count", "value": 1 }
iot/dev/Default/data { "type": "count", "value": 2 }
iot/dev/Default/data { "type": "count", "value": 3 }
iot/dev/Default/data { "type": "count", "value": 4 }
iot/dev/Default/data { "type": "count", "value": 5 }
iot/dev/Default/data { "type": "count", "value": 6 }
iot/dev/Default/data { "type": "count", "value": 7 }
iot/dev/Default/data { "type": "count", "value": 8 }
iot/dev/Default/data { "type": "count", "value": 9 }
iot/dev/Default/data { "type": "count", "value": 10 }
iot/dev/Default/data { "type": "count", "value": 11 }
iot/dev/Default/data { "type": "count", "value": 12 }
iot/dev/Default/data { "type": "count", "value": 13 }
```

## Additional Tools

You can run additional `idf.py` custom command for some additional tasks, like:

* For code [**Formatting**](https://clang.llvm.org/docs/ClangFormat.html) `idf.py format`.
`clang-format` version 13.0.0 is required.

* For code [**Static Analyze**](https://cppcheck.sourceforge.io) `idf.py cppheck`.
`cppcheck` version 2.6 is required.
