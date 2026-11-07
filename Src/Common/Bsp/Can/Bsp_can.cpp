#include "Bsp_can.hpp"
#include "Soc.hpp"

extern canHandle HCAN1;
extern canHandle HCAN2;
#if SOC_CAN_NUM == 3
extern canHandle HCAN3;
#endif

HAL_StatusTypeDef Can::init() {
    HAL_StatusTypeDef result = HAL_OK;
    result = initSelf(&HCAN1, RX_FIFO0);
    return result;
}

HAL_StatusTypeDef Can::registerCallback(canHandle *_hcan, uint32_t _stdid, callback _pCallback)
{
    struct Handler_s temp = { .stdid = _stdid, .func = _pCallback };
    if (_hcan == &HCAN1) {
        if (can1cnt < MAX_RECV_DEVICE) {
            cbTable1[can1cnt] = temp;
            can1cnt++;
            return HAL_OK;
        } else {
            return HAL_ERROR;
        }
    }
    
    return HAL_ERROR;
}

void Can::unregisterCallback(canHandle *_hcan, uint32_t _stdid)
{
    if (_hcan == &HCAN1) {
        for (uint8_t i = 0; i < can1cnt; i++) {
            if (cbTable1[i].stdid == _stdid) {
                cbTable1[i].func = nullptr;
                break;
            }
        }
    }
}

#if defined(SOC_CAN)
HAL_StatusTypeDef canFilterInit(CAN_HandleTypeDef *_can, uint32_t _fifo) {
    HAL_StatusTypeDef result = HAL_OK;
    if (_can != &HCAN1 || (_fifo != RX_FIFO0 && _fifo != RX_FIFO1)) {
        return HAL_ERROR;
    }
    CAN_FilterTypeDef canFilter = {};
    canFilter.FilterActivation = ENABLE;
    canFilter.FilterMode = CAN_FILTERMODE_IDMASK;
    canFilter.FilterScale = CAN_FILTERSCALE_32BIT;
    canFilter.FilterFIFOAssignment = _fifo;
    canFilter.FilterIdHigh = 0x0000;
    canFilter.FilterIdLow = 0x0000;
    canFilter.FilterMaskIdHigh = 0x0000;
    canFilter.FilterMaskIdLow = 0x0000;
    canFilter.SlaveStartFilterBank = 14;
    if (_can == &HCAN1) {
        canFilter.FilterBank = 0;
    }
    result = HAL_CAN_ConfigFilter(_can, &canFilter);
    if (result != HAL_OK) {
        return result;
    }
    if (_fifo == RX_FIFO0) {
        result = HAL_CAN_ActivateNotification(_can, CAN_IT_RX_FIFO0_MSG_PENDING);
    } else if (_fifo == RX_FIFO1) {
        result = HAL_CAN_ActivateNotification(_can, CAN_IT_RX_FIFO1_MSG_PENDING);
    }
    return result;
}

HAL_StatusTypeDef Can::initSelf(canHandle *_hcan, uint32_t _fifo) {
    HAL_StatusTypeDef result = HAL_OK;
    result = canFilterInit(_hcan, _fifo);
    if (result != HAL_OK) {
        return result;
    }
    __HAL_CAN_ENABLE_IT(_hcan, CAN_IT_BUSOFF);
    result = HAL_CAN_Start(_hcan);
    return result;
}

HAL_StatusTypeDef Can::transmitData(canHandle *_hcan, uint16_t _stdid, uint8_t *_txData, uint32_t _len) {
    CAN_TxHeaderTypeDef txHeader;
    uint32_t canMailbox;
    txHeader.DLC = _len;
    txHeader.IDE = CAN_ID_STD;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.StdId = _stdid;
    txHeader.TransmitGlobalTime = DISABLE;
    return HAL_CAN_AddTxMessage(_hcan, &txHeader, _txData, &canMailbox);

}

void Can::callbackFromISR(canHandle *_hcan, uint32_t _rxFifo)
{
    uint8_t rxData[8] = {};
    canHeader rxHeader = {};
    if (HAL_CAN_GetRxMessage(_hcan, _rxFifo, &rxHeader, rxData) != HAL_OK) {
        return;
    }
    if (rxHeader.IDE != CAN_ID_STD || rxHeader.RTR != CAN_RTR_DATA || rxHeader.DLC > 8) {
        return;
    }
#if BSP_USE_LED
    if (_hcan == &HCAN1 && rxHeader.StdId == 0x100 && rxHeader.DLC != 2) {
        return;
    }
#endif
    if (_hcan == &HCAN1) {
        for (uint8_t i = 0; i < can1cnt; i++) {
            if (cbTable1[i].stdid == rxHeader.StdId) {
                if (cbTable1[i].func) {
                    cbTable1[i].func(rxData);
                }
                break;
            }
        }
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *_hcan) { Can::instance().callbackFromISR(_hcan, RX_FIFO0); }

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *_hcan) { Can::instance().callbackFromISR(_hcan, RX_FIFO1); }

#endif
