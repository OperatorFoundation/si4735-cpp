//
// Created by Dr. Brandon Wiley on 11/13/25.
//

#ifndef SI4735_CPP_SI4735ARDUINO_H
#define SI4735_CPP_SI4735ARDUINO_H

#include <si4735-cpp.h>

class SI4735Arduino : public SI4735Base
{
  public:
    void setup(uint8_t resetPin, uint8_t ctsIntEnable, uint8_t defaultFunction, uint8_t audioMode, uint8_t clockType, uint8_t gpo2Enable);
    bool downloadPatch(const uint8_t *ssb_patch_content, const uint16_t ssb_patch_content_size);
    bool downloadCompressedPatch(const uint8_t *ssb_patch_content, const uint16_t ssb_patch_content_size, const uint16_t *cmd_0x15, const int16_t cmd_0x15_size);

    /**
     * @ingroup group18 MCU External Audio Mute
     *
     * @brief Sets the Audio Mute Mcu Pin
     * @details This function sets the mcu digital pin you want to use to control the external audio mute circuit.
     * @details Some users may be uncomfortable with the loud popping of the speaker during some transitions caused by some SI47XX commands.
     * @details This problem occurs during the transition from the power down to power up.
     * @details For example, when the user changes bands (FM to AM or AM to FM), the Si47XX devices must be powered down and powered up again.
     * @details If you have a mute circuit attached to a pin on the MCU, then you can control the mute circuit from the MCU with this function.
     *
     * @see setHardwareAudioMute
     * @param pin if 0 or greater, sets the MCU digital pin that controls the external circuit.
     */
    inline void setAudioMuteMcuPin(int8_t pin)
    {
      audioMuteMcuPin = pin;
      pinMode(audioMuteMcuPin, OUTPUT);
    };

    /**
     * @ingroup group18 MCU External Audio Mute
     *
     * @brief Sets the Hardware Audio Mute
     * @details Turns the Hardware audio mute on or off
     *
     * @see setAudioMuteMcuPin
     *
     * @param on  True or false
     */
    inline void setHardwareAudioMute(bool on)
    {
      digitalWrite(audioMuteMcuPin, on);
      clock.waitMicroseconds(300);
    }

    /**
     * @ingroup group06 RESET
     *
     * @brief Reset the SI473X
     *
     * @see Si47XX PROGRAMMING GUIDE; AN332 (REV 1.0);
     */
    void SI4735Base::reset()
    {
      pinMode(resetPin, OUTPUT);
      clock.wait(10);
      digitalWrite(resetPin, LOW);
      clock.wait(10);
      digitalWrite(resetPin, HIGH);
      clock.wait(10);
    }

    void radioPowerUp(void);
    void powerDown(void);
};

#endif //SI4735_CPP_SI4735ARDUINO_H