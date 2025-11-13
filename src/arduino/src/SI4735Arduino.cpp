//
// Created by Dr. Brandon Wiley on 11/13/25.
//

#include "SI4735Arduino.h"

#include <Arduino.h>

/**
 * @ingroup   group07 Device start up
 *
 * @brief Starts the Si473X device.
 *
 * @details Use this function to start the device up with the parameters shown below.
 * @details If the audio mode parameter is not entered, analog mode will be considered.
 * @details You can use any Arduino digital pin. Be sure you are using less than 3.6V on Si47XX RST pin.
 *
 * ATTENTION: The document AN383; "Si47XX ANTENNA, SCHEMATIC, LAYOUT, AND DESIGN GUIDELINES"; rev 0.8; page 6; there is the following note:
 *            Crystal and digital audio mode cannot be used at the same time. Populate R1 and remove C10, C11, and X1 when using digital audio.
 *
 * @param resetPin Digital Arduino Pin used to RESET de Si47XX device.
 * @param ctsIntEnable CTS Interrupt Enable.
 * @param defaultFunction is the mode you want the receiver starts.
 * @param audioMode default SI473X_ANALOG_AUDIO (Analog Audio). Use SI473X_ANALOG_AUDIO or SI473X_DIGITAL_AUDIO.
 * @param clockType 0 = Use external RCLK (crystal oscillator disabled); 1 = Use crystal oscillator
 * @param gpo2Enable GPO2OE (GPO2 Output) 1 = Enable; 0 Disable (defult)
 */
void SI4735Base::setup(uint8_t resetPin, uint8_t ctsIntEnable, uint8_t defaultFunction, uint8_t audioMode, uint8_t clockType, uint8_t gpo2Enable)
{
    this->resetPin = resetPin;
    this->ctsIntEnable = (ctsIntEnable != 0) ? 1 : 0; // Keeps old versions of the sketches running
    this->gpo2Enable = gpo2Enable;
    this->currentAudioMode = audioMode;

    // Set the initial SI473X behavior
    // CTSIEN   interruptEnable -> Interrupt anabled or disable;
    // GPO2OEN  1 -> GPO2 Output Enable;
    // PATCH    0 -> Boot normally;
    // XOSCEN   clockType -> Use external crystal oscillator (XOSCEN_CRYSTAL) or reference clock (XOSCEN_RCLK);
    // FUNC     defaultFunction = 0 = FM Receive; 1 = AM (LW/MW/SW) Receiver.
    // OPMODE   SI473X_ANALOG_AUDIO or SI473X_DIGITAL_AUDIO.
    setPowerUp(ctsIntEnable, gpo2Enable, 0, clockType, defaultFunction, audioMode);

    if (audioMuteMcuPin >= 0)
        setHardwareAudioMute(true); // If you are using external citcuit to mute the audio, it turns the audio mute

    reset();

    radioPowerUp();
    setVolume(30); // Default volume level.
    getFirmware();
}

/**
 * @ingroup group17 Patch and SSB support
 *
 * @brief Transfers the content of a patch stored in a array of bytes to the SI4735 device.
 *
 * @details You must mount an array as shown below and know the size of that array as well.
 *
 *  @details Patches for the SI4735 are distributed in binary
 *   form and are transferred to the internal RAM of the device by the host MCU (in this case, Arduino boards).
 *   Since the RAM is volatile memory, the patch stored on the device gets lost when you turn off
 *   the system. Consequently, the content of the patch has to be transferred to the device every
 *   time the device is powered up.
 *
 *  @details The disadvantage of this approach is the amount of memory used by the patch content.
 *  This may limit the use of other radio functions you want implemented in Arduino.
 *
 *  @details Example of content:
 *  @code
 *  const PROGMEM uint8_t ssb_patch_content_full[] =
 *   { // SSB patch for whole SSBRX full download
 *       0x15, 0x00, 0x0F, 0xE0, 0xF2, 0x73, 0x76, 0x2F,
 *       0x16, 0x6F, 0x26, 0x1E, 0x00, 0x4B, 0x2C, 0x58,
 *       0x16, 0xA3, 0x74, 0x0F, 0xE0, 0x4C, 0x36, 0xE4,
 *          .
 *          .
 *          .
 *       0x16, 0x3B, 0x1D, 0x4A, 0xEC, 0x36, 0x28, 0xB7,
 *       0x16, 0x00, 0x3A, 0x47, 0x37, 0x00, 0x00, 0x00,
 *       0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9D, 0x29};
 *
 *  const int size_content_full = sizeof ssb_patch_content_full;
 *  @endcode
 *
 * @see Si47XX PROGRAMMING GUIDE; ;AN332 (REV 1.0) pages 64 and 215-220.
 *
 *  @param ssb_patch_content point to array of bytes content patch.
 *  @param ssb_patch_content_size array size (number of bytes). The maximum size allowed for a patch is 15856 bytes
 *
 *  @return false if an error is found.
 */
bool SI4735::downloadPatch(const uint8_t *ssb_patch_content, const uint16_t ssb_patch_content_size)

/**
 * @ingroup group17 Patch and SSB support
 *
 * @brief   Deal with compressed SSB patch
 * @details It works like the downloadPatch method but requires less memory to store the patch.
 * @details Transfers the content of a patch stored in a compressed array of bytes to the SI4735 device.
 * @details In the patch_init.h and patch_full.h files, the first byte of each line begins with either a 0x15 or 0x16 value
 * @details To shrink the original patch size stored on the master MCU (Arduino), the first byte
 * @details is omitted and a new array is added to indicate which lines begin with the value 0x15.
 * @details For the other lines, the downloadCompressedPatch method will insert the value 0x16.
 * @details The value 0x16 occurs on most lines in the patch. This approach will save about 1K of memory.
 * @details The example code below shows how to use compressed SSB patch.
 * @code
 *   #include <patch_ssb_compressed.h> // SSB patch for whole SSBRX initialization string
 *   const uint16_t size_content = sizeof ssb_patch_content; // See ssb_patch_content.h
 *   const uint16_t cmd_0x15_size = sizeof cmd_0x15;         // Array of lines where the 0x15 command occurs in the patch content.
 *
 *   void loadSSB()
 *   {
 *     .
 *     .
 *     rx.setI2CFastModeCustom(500000);
 *     rx.queryLibraryId();
 *     rx.patchPowerUp();
 *     clock.wait(50);
 *     rx.downloadCompressedPatch(ssb_patch_content, size_content, cmd_0x15, cmd_0x15_size);
 *     rx.setSSBConfig(bandwidthSSB[bwIdxSSB].idx, 1, 0, 1, 0, 1);
 *     rx.setI2CStandardMode();
 *     .
 *     .
 *   }
 * @endcode
 * @see  downloadPatch
 * @see  patch_ssb_compressed.h, patch_init.h, patch_full.h
 * @see  SI47XX_03_ALL_IN_ONE_NEW_INTERFACE_V15.ino
 * @see  SI47XX_09_NOKIA_5110/ALL_IN_ONE_7_BUTTONS/ALL_IN_ONE_7_BUTTONS.ino
 * @param ssb_patch_content         point to array of bytes content patch.
 * @param ssb_patch_content_size    array size (number of bytes). The maximum size allowed for a patch is 15856 bytes
 * @param cmd_0x15                  Array of lines where the first byte of each patch content line is 0x15
 * @param cmd_0x15_size             Array size
 */
bool SI4735::downloadCompressedPatch(const uint8_t *ssb_patch_content, const uint16_t ssb_patch_content_size, const uint16_t *cmd_0x15, const int16_t cmd_0x15_size)
{
    uint8_t cmd, content;
    uint16_t command_line = 0;
    // Send patch to the SI4735 device
    for (uint16_t offset = 0; offset < ssb_patch_content_size; offset += 7)
    {
        // Checks if the current line starts with 0x15
        cmd = 0x16;
        for (uint16_t i = 0; i < cmd_0x15_size / sizeof(uint16_t); i++)
        {
            if (pgm_read_word_near(cmd_0x15 + i) == command_line)
            { // it needs performance improvement: save the last "i" value to be used next time
                cmd = 0x15;
                break;
            }
        }
        i2c.beginTransmission(deviceAddress);
        i2c.write(cmd);
        for (uint16_t i = 0; i < 7; i++)
        {
            content = pgm_read_byte_near(ssb_patch_content + (i + offset));
            i2c.write(content);
        }
        i2c.endTransmission();
        delayMicroseconds(MIN_DELAY_WAIT_SEND_LOOP); // Need check the minimum value
        command_line++;
    }
    delayMicroseconds(250);
    return true;
}
{
    uint8_t content;
    // Send patch to the SI4735 device
    for (uint16_t offset = 0; offset < ssb_patch_content_size; offset += 8)
    {
        i2c.beginTransmission(deviceAddress);
        for (uint16_t i = 0; i < 8; i++)
        {
            content = pgm_read_byte_near(ssb_patch_content + (i + offset));
            i2c.write(content);
        }
        i2c.endTransmission();

        // Testing download performance
        // approach 1 - Faster - less secure (it might crash in some architectures)
        delayMicroseconds(MIN_DELAY_WAIT_SEND_LOOP); // Need check the minimum value

        // approach 2 - More control. A little more secure than approach 1
        /*
        do
        {
            delayMicroseconds(150); // Minimum delay founded (Need check the minimum value)
            i2c.requestFrom(deviceAddress, 1);
        } while (!(i2c.read() & B10000000));
        */

        // approach 3 - same approach 2
        // waitToSend();

        // approach 4 - safer
        /*
        waitToSend();
        uint8_t cmd_status;
        // Uncomment the lines below if you want to check erro.
        i2c.requestFrom(deviceAddress, 1);
        cmd_status = i2c.read();
        // The SI4735 issues a status after each 8 byte transfered.
        // Just the bit 7 (CTS) should be seted. if bit 6 (ERR) is seted, the system halts.
        if (cmd_status != 0x80)
           return false;
        */
    }
    delayMicroseconds(250);
    return true;
}

/**
 * @ingroup group07 Device Power Up
 *
 * @brief Powerup the Si47XX
 *
 * @details Before call this function call the setPowerUp to set up the parameters.
 *
 * @details Parameters you have to set up with setPowerUp
 *
 * | Parameter | Description |
 * | --------- | ----------- |
 * | CTSIEN    | Interrupt anabled or disabled |
 * | GPO2OEN   | GPO2 Output Enable or disabled |
 * | PATCH     | Boot normally or patch |
 * | XOSCEN    | 0 (XOSCEN_RCLK) = external active crystal oscillator. 1 (XOSCEN_CRYSTAL) = passive crystal oscillator;  |
 * | FUNC      | defaultFunction = 0 = FM Receive; 1 = AM (LW/MW/SW) Receiver |
 * | OPMODE    | SI473X_ANALOG_AUDIO (B00000101) or SI473X_DIGITAL_AUDIO (B00001011) |
 *
 * ATTENTION: The document AN383; "Si47XX ANTENNA, SCHEMATIC, LAYOUT, AND DESIGN GUIDELINES"; rev 0.8; page 6; there is the following note:
 *            Crystal and digital audio mode cannot be used at the same time. Populate R1 and remove C10, C11, and X1 when using digital audio.
 *
 * @see setMaxDelaySetFrequency()
 * @see MAX_DELAY_AFTER_POWERUP
 * @see XOSCEN_CRYSTAL
 * @see XOSCEN_RCLK
 * @see  SI4735Base::setPowerUp()
 * @see  Si47XX PROGRAMMING GUIDE; AN332 (REV 1.0); pages 64, 129
 */
void SI4735Arduino::radioPowerUp(void)

/**
 * @ingroup group07 Device Power Down
 *
 * @brief Moves the device from powerup to powerdown mode.
 *
 * @details After Power Down command, only the Power Up command is accepted.
 *
 * @see Si47XX PROGRAMMING GUIDE; AN332 (REV 1.0); pages 67, 132
 * @see radioPowerUp()
 */
void SI4735Base::powerDown(void)
{
    // Turns the external mute circuit on
    if (audioMuteMcuPin >= 0)
        setHardwareAudioMute(true);

    waitToSend();
    i2c.beginTransmission(deviceAddress);
    i2c.write(POWER_DOWN);
    i2c.endTransmission();
    clock.waitMicroseconds(2500);
}
{
    // clock.waitMicroseconds(1000);
    waitToSend();
    i2c.beginTransmission(deviceAddress);
    i2c.write(POWER_UP);
    i2c.write(powerUp.raw[0]); // Content of ARG1
    i2c.write(powerUp.raw[1]); // COntent of ARG2
    i2c.endTransmission();
    // Delay at least 500 ms between powerup command and first tune command to wait for
    // the oscillator to stabilize if XOSCEN is set and crystal is used as the RCLK.
    waitToSend();
    clock.wait(maxDelayAfterPowerUp);

    // Turns the external mute circuit off
    if (audioMuteMcuPin >= 0)
        setHardwareAudioMute(false);

    if (this->currentClockType == XOSCEN_RCLK)
    {
        setRefClock(this->refClock);
        setRefClockPrescaler(this->refClockPrescale, this->refClockSourcePin);
    }
}
