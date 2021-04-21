# IoT Thing: 4 Inputs with counter

## Introduction

This is web connected 4 inputs module.

## Dependencies

Requires:

 * [Web Thing Server](https://github.com/KrzysztofZurek1973/iot_components/tree/master/web_thing_server)


## Features

4 inputs webThing has @type `MultiLevelSwitch` and following properties:

* **Input-1** up to **Input-4**, indicates if input is in high (`open`) or low (`closed`) state, @type: `OpenProperty`
* **counter**, shows how many times inputs has been changed, max = 1000, @type: `LevelProperty`
* **ON-OFF**, ON: at least one input is high, @type: `OnOffProperty`

## Documentation

How to start the node see [iot_web_thing_example](https://github.com/KrzysztofZurek1973/iot_web_thing_example#power-up).

### Configuration

GPIO number can be set in `idf.py menuconfig -> Inputs config` (see [iot_web_thing_example](https://github.com/KrzysztofZurek1973/iot_web_thing_example)).

Default input GPIOs:

* INPUT-1: GPIO32
* INPUT-2: GPIO33
* INPUT-3: GPIO25
* INPUT-4: GPIO26

### Gateway web interface

![GUI](./images/4_inputs_gui.png)

To trigger an action in another webThing (e.g. set output-1 in `4-output` high when input-1 drops low) use Gateway rule:

![Rule](./images/4_inputs_rule.png)

## Source Code

The source is available from [GitHub](https://github.com/KrzysztofZurek1973/webthing-4-inputs).

## Links

* [WebThing Gateway](https://webthings.io/gateway/) - https://webthings.io/gateway/
* [Web Thing API](https://webthings.io/api/) - https://webthings.io/api/
* [esp-idf](https://github.com/espressif/esp-idf) - https://github.com/espressif/esp-idf


## License

The code in this project is licensed under the MIT license - see LICENSE for details.

## Authors

* **Krzysztof Zurek** - [kz](https://github.com/KrzysztofZurek1973)

