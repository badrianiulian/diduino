# Diduino programmer - with English language

![Diduino board](/img/board.png "Diduino")

Project of a programmer for EEPROM chips: K155RE3, KR556RT4, KR556RT14, KR556RT5 and their analogues. Based on the following projects:
* [ProgRT programmer project](https://zx-pk.ru/threads/15617-programmator-ppzu-155re3-556rt4-i-drugikh.html "Link to the ZX-PK forum")
* [Programmer project from walhi](https://github.com/walhi/arduino_eprom27_programmer "Github link")

Schematic diagram:

![Schematic diagram](/img/scheme.png "Schematic diagram")

Schematic and board project: <https://easyeda.com/naym1993/prog_did>

The operating principle of the regulator on the XL60009 was discussed [in this video](https://youtu.be/bcgiW_VbnP0 "Link to YouTube")

**In PCB version 1.0 there was an error in the board layout!!**

It is necessary to change the drain source in the power transistor.

![Refinement of board version 1.0](/img/fix_for_1.0.jpg "Refinement")

I discuss this issue in more detail in the video: <https://youtu.be/s_gQgh_qvWw>

**A flaw was also found in version 1.1 of the printed circuit board!!! Thanks PticDado!**

Resistor assembly R18 is not connected to ground. The contact is just hanging in the air, you need to throw the jumper to the ground (it is around the contact).
In the easyEDA project, the printed circuit board has already been corrected.

## Diduino programmer

Project of the programmer of ROM chips: K155RE3, KR556RT4, KR556RT14, KR556RT5 and their analogs. It is based on the following projects:
* [ProgRT programmer project](https://zx-pk.ru/threads/15617-programmator-ppzu-155re3-556rt4-i-drugikh.html "Link to ZX-PK forum")
* [Programmer project by walhi](https://github.com/walhi/arduino_eprom27_programmer "Link to github")

Circuit and board design: <https://easyeda.com/naym1993/prog_did>

The principle of the XL60009 regulator was discussed [in this video](https://youtu.be/bcgiW_VbnP0 "Youtube link")

Fixes are provided by Vali. I just compiled them ˆ_ˆ
