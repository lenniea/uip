#ifndef __UDPCAM_H__
#define __UDPCAM_H__

enum cam_status
{
    CAM_OK                            = 0,     /*!< Camera ok */

    /* Camera Errors (must be positive to fit in status byte) */
    CAM_ERROR                         = -1,    /*!< Camera error */
    CAM_NOT_READY                     = -2,    /*!< Camera not ready */
    CAM_RANGE_ERROR                   = -3,    /*!< Camera range error */
    CAM_CHECKSUM_ERROR                = -4,    /*!< Camera checksum eror */
    CAM_UNDEFINED_PROCESS_ERROR       = -5,    /*!< Camera undefined process error */
    CAM_UNDEFINED_FUNCTION_ERROR      = -6,    /*!< Camera undefined function error */
    CAM_TIMEOUT_ERROR                 = -7,    /*!< Camera timeout error */
    CAM_BYTE_COUNT_ERROR              = -9,    /*!< Camera byte count error */
    CAM_FEATURE_NOT_ENABLED           = -10    /*!< Camera feature not enabled */
};

short CamCommand(short func, short count, short data[]);

#endif /* __UDPCAM_H__ */
