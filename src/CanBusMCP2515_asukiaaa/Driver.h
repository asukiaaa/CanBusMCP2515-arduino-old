#pragma once

#include <Arduino.h>
#include <SPI.h>

#include <CanBusData_asukiaaa.hpp>

#include "Settings.h"
#include "ACANBuffer16.h"
#include "MCP2515ReceiveFilters.h"

// #define SERIAL_DEBUG Serial

namespace CanBusMCP2515_asukiaaa {

typedef void (*ACANCallBackRoutine)(
    const CanBusData_asukiaaa::Frame &inMessage);

class Driver {
 public:
  Driver(const uint8_t inCS,    // CS input of MCP2515
         const uint8_t inINT);  // INT output of MCP2515
  uint16_t begin(const Settings &inSettings,
                 void (*inInterruptServiceRoutine)(void));
  uint16_t begin(const Settings &inSettings,
                 void (*inInterruptServiceRoutine)(void),
                 const ACAN2515Mask inRXM0,
                 const ACAN2515AcceptanceFilter inAcceptanceFilters[],
                 const uint8_t inAcceptanceFilterCount);
  uint16_t begin(const Settings &inSettings,
                 void (*inInterruptServiceRoutine)(void),
                 const ACAN2515Mask inRXM0, const ACAN2515Mask inRXM1,
                 const ACAN2515AcceptanceFilter inAcceptanceFilters[],
                 const uint8_t inAcceptanceFilterCount);
  static const uint16_t kNoMCP2515 = 1 << 0;
  static const uint16_t kTooFarFromDesiredBitRate = 1 << 1;
  static const uint16_t kInconsistentBitRateSettings = 1 << 2;
  static const uint16_t kINTPinIsNotAnInterrupt = 1 << 3;
  static const uint16_t kISRIsNull = 1 << 4;
  static const uint16_t kRequestedModeTimeOut = 1 << 5;
  static const uint16_t kAcceptanceFilterArrayIsNULL = 1 << 6;
  static const uint16_t kOneFilterMaskRequiresOneOrTwoAcceptanceFilters = 1
                                                                          << 7;
  static const uint16_t kTwoFilterMasksRequireThreeToSixAcceptanceFilters =
      1 << 8;
  static const uint16_t kCannotAllocateReceiveBuffer = 1 << 9;
  static const uint16_t kCannotAllocateTransmitBuffer0 = 1 << 10;
  static const uint16_t kCannotAllocateTransmitBuffer1 = 1 << 11;
  static const uint16_t kCannotAllocateTransmitBuffer2 = 1 << 12;
  static const uint32_t kISRNotNullAndNoIntPin = 1 << 13;
  uint16_t changeModeOnTheFly(
      const Settings::RequestedMode inRequestedMode);
  uint16_t setFiltersOnTheFly(void);
  uint16_t setFiltersOnTheFly(
      const ACAN2515Mask inRXM0,
      const ACAN2515AcceptanceFilter inAcceptanceFilters[],
      const uint8_t inAcceptanceFilterCount);
  uint16_t setFiltersOnTheFly(
      const ACAN2515Mask inRXM0, const ACAN2515Mask inRXM1,
      const ACAN2515AcceptanceFilter inAcceptanceFilters[],
      const uint8_t inAcceptanceFilterCount);
  void end(void);
  bool available(void);
  bool receive(CanBusData_asukiaaa::Frame* outFrame);
  typedef void (*tFilterMatchCallBack)(const uint8_t inFilterIndex);
  bool dispatchReceivedMessage(
      const tFilterMatchCallBack inFilterMatchCallBack = NULL);
  void isr(void);
  bool isr_core(void);

#ifdef ARDUINO_ARCH_ESP32
  SemaphoreHandle_t mISRSemaphore;
#endif

  inline uint16_t receiveBufferSize(void) const {
    return mReceiveBuffer.size();
  }

  inline uint16_t receiveBufferCount(void) const {
    return mReceiveBuffer.count();
  }

  inline uint16_t receiveBufferPeakCount(void) const {
    return mReceiveBuffer.peakCount();
  }

  bool tryToSend(const CanBusData_asukiaaa::Frame &inMessage);

  inline uint16_t transmitBufferSize(const uint8_t inIndex) const {
    return mTransmitBuffer[inIndex].size();
  }
  inline uint16_t transmitBufferCount(const uint8_t inIndex) const {
    return mTransmitBuffer[inIndex].count();
  }
  inline uint16_t transmitBufferPeakCount(const uint8_t inIndex) const {
    return mTransmitBuffer[inIndex].peakCount();
  }

  void poll(void);

  uint8_t receiveErrorCounter(void);
  uint8_t transmitErrorCounter(void);
  uint8_t errorFlagRegister(void);

 private:
  ACANBuffer16 mTransmitBuffer[3];
  bool mTXBIsFree[3];
  ACANCallBackRoutine mCallBackFunctionArray[6];
  ACANBuffer16 mReceiveBuffer;
  void handleTXBInterrupt(const uint8_t inTXB);
  void handleRXBInterrupt(void);
  SPIClass* mSpi = NULL;
  const SPISettings mSPISettings;
  const uint8_t mCS;
  const uint8_t mINT;

  void internalSendMessage(const CanBusData_asukiaaa::Frame &inFrame,
                           const uint8_t inTXB);
  inline void select(void) { digitalWrite(mCS, LOW); }
  inline void unselect(void) { digitalWrite(mCS, HIGH); }
  uint16_t beginWithoutFilterCheck(
      const Settings &inSettings,
      void (*inInterruptServiceRoutine)(void), const ACAN2515Mask inRXM0,
      const ACAN2515Mask inRXM1,
      const ACAN2515AcceptanceFilter inAcceptanceFilters[],
      const uint8_t inAcceptanceFilterCount);
  uint16_t internalBeginOperation(
      const Settings &inSettings, const ACAN2515Mask inRXM0,
      const ACAN2515Mask inRXM1,
      const ACAN2515AcceptanceFilter inAcceptanceFilters[],
      const uint8_t inAcceptanceFilterCount);
  void write2515Register(const uint8_t inRegister, const uint8_t inValue);
  uint8_t read2515Register(const uint8_t inRegister);
  uint8_t read2515Status(void);
  uint8_t read2515RxStatus(void);
  void bitModify2515Register(const uint8_t inRegister, const uint8_t inMask,
                             const uint8_t inData);
  void setupMaskRegister(const ACAN2515Mask inMask, const uint8_t inRegister);
  uint16_t setRequestedMode(const uint8_t inCANControlRegister);
  uint16_t internalSetFiltersOnTheFly(
      const ACAN2515Mask inRXM0, const ACAN2515Mask inRXM1,
      const ACAN2515AcceptanceFilter inAcceptanceFilters[],
      const uint8_t inAcceptanceFilterCount);
  Driver(const Driver &) = delete;
  Driver &operator=(const Driver &) = delete;
};

};  // namespace CanBusMCP2515_asukiaaa
