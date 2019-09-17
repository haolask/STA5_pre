/*
 * (C) COPYRIGHT 2011 HANTRO PRODUCTS
 *
 * Please contact: hantro-support@verisilicon.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

/*------------------------------------------------------------------------------
--
--  Description : API of H.264 Decoder
--
------------------------------------------------------------------------------*/

/*!\file
 * \brief Describes the H.264 decoder algorithm interface to applications.
 *
 * This file describes the interface between an application and a
 * video decoder.
 *
 */

#ifndef __H264DECAPI_H__
#define __H264DECAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "basetype.h"
#include "decapicommon.h"

/*!\defgroup H264API H.264 Decoder API */

/*! \addtogroup H264API
 *  @{
 */

/*------------------------------------------------------------------------------
    API type definitions
------------------------------------------------------------------------------*/

    /*!\enum H264DecRet_
     * Return values for API
     *
     * \typedef H264DecRet
     * A typename for #H264DecRet_
     */
    typedef enum H264DecRet_
    {
        /** Success */
        H264DEC_OK = 0,  /**<\hideinitializer */
        /** Stream processed */
        H264DEC_STRM_PROCESSED = 1,  /**<\hideinitializer */
        /** Picture available for output */
        H264DEC_PIC_RDY = 2,  /**<\hideinitializer */
        /** Picture decoded */
        H264DEC_PIC_DECODED = 3,  /**<\hideinitializer */
        /** New stream headers decoded */
        H264DEC_HDRS_RDY = 4,  /**<\hideinitializer */
        /** Advanced coding tools detected in stream */
        H264DEC_ADVANCED_TOOLS = 5,  /**<\hideinitializer */
        /** Output pictures must be retrieved before continuing decode */
        H264DEC_PENDING_FLUSH = 6,  /**<\hideinitializer */
        /** Skipped decoding non-reference picture */
        H264DEC_NONREF_PIC_SKIPPED = 7, /**<\hideinitializer */
        /** End-of-stream state set in the decoder */
        H264DEC_END_OF_STREAM = 8, /**<\hideinitializer */

        H264DEC_PARAM_ERROR = -1, /**<\hideinitializer */
        H264DEC_STRM_ERROR = -2, /**<\hideinitializer */
        H264DEC_NOT_INITIALIZED = -3, /**<\hideinitializer */
        H264DEC_MEMFAIL = -4, /**<\hideinitializer */
        H264DEC_INITFAIL = -5, /**<\hideinitializer */
        H264DEC_HDRS_NOT_RDY = -6, /**<\hideinitializer */
        H264DEC_STREAM_NOT_SUPPORTED = -8, /**<\hideinitializer */

        H264DEC_HW_RESERVED = -254, /**<\hideinitializer */
        H264DEC_HW_TIMEOUT = -255, /**<\hideinitializer */
        H264DEC_HW_BUS_ERROR = -256, /**<\hideinitializer */
        H264DEC_SYSTEM_ERROR = -257, /**<\hideinitializer */
        H264DEC_DWL_ERROR = -258, /**<\hideinitializer */

        H264DEC_EVALUATION_LIMIT_EXCEEDED = -999, /**<\hideinitializer */
        H264DEC_FORMAT_NOT_SUPPORTED = -1000 /**<\hideinitializer */
    } H264DecRet;

    /*!\enum H264DecOutFormat_
     *  Decoder output picture format.
     *
     * \typedef H264DecOutFormat
     * A typename for #H264DecOutFormat_.
     */
    typedef enum H264DecOutFormat_
    {
        H264DEC_SEMIPLANAR_YUV420 = 0x020001, /**<\hideinitializer */
        H264DEC_TILED_YUV420 = 0x020002, /**<\hideinitializer */
        /** Monochrome YUV */
        H264DEC_YUV400 = 0x080000 /**<\hideinitializer */
    } H264DecOutFormat;

    /*!\typedef H264DecInst
     * \brief Decoder instance
     */
    typedef const void *H264DecInst;

    /*!\struct H264DecInput_
     * \brief Decode input structure
     *
     * \typedef H264DecInput
     * A typename for #H264DecInput_.
     */
    typedef struct H264DecInput_
    {
        const u8 *pStream;   /**< Pointer to the input buffer*/
        u32 streamBusAddress; /**< DMA bus address of the input buffer */
        u32 dataLen;          /**< Number of bytes to be decoded         */
        u32 picId;            /**< Identifier for the next picture to be decoded */
        u32 skipNonReference; /**< Flag to enable decoder to skip non-reference
                               * frames to reduce processor load */
        void *pUserData; /* user data to be passed in multicore callback */
    } H264DecInput;

    /*!\struct H264DecOutput_
     * \brief Decode output structure
     *
     * \typedef H264DecOutput
     * A typename for #H264DecOutput_.
     */
    typedef struct H264DecOutput_
    {
        u8 *pStrmCurrPos;    /**< Pointer to stream position where decoding ended */
        u32 strmCurrBusAddress; /**< DMA bus address location where the decoding ended */
        u32 dataLeft;        /**< how many bytes left unprocessed */
    } H264DecOutput;

    /*!\struct H264CropParams_
     * \brief Picture cropping information
     *
     * \typedef H264CropParams
     * A typename for #H264CropParams_.
     */
    typedef struct H264CropParams_
    {
        u32 cropLeftOffset;
        u32 cropOutWidth;
        u32 cropTopOffset;
        u32 cropOutHeight;
    } H264CropParams;

    /*!\struct H264DecPicture_
     * \brief Decoded picture information
     *
     * Parameters of a decoded picture, filled by H264DecNextPicture().
     *
     * \typedef H264DecPicture
     * A typename for #H264DecPicture_.
     */
    typedef struct H264DecPicture_
    {
        u32 picWidth;        /**< pixels width of the picture as stored in memory */
        u32 picHeight;       /**< pixel height of the picture as stored in memory */
        H264CropParams cropParams;  /**< cropping parameters */
        const u32 *pOutputPicture;  /**< Pointer to the picture data */
        u32 outputPictureBusAddress;    /**< DMA bus address of the output picture buffer */
        u32 picId;           /**< Identifier of the picture to be displayed */
        u32 picCodingType[2];   /**< Picture coding type */
        u32 isIdrPicture[2];    /**< Indicates if picture is an IDR picture */
        u32 nbrOfErrMBs;     /**< Number of concealed MB's in the picture  */
        u32 interlaced;      /**< flag, non-zero for interlaced picture */
        u32 fieldPicture;    /**< flag, non-zero if interlaced and only one field present */
        u32 topField;        /**< flag, if only one field, non-zero signals TOP field otherwise BOTTOM */
        u32 viewId;          /**< Identifies the view to which the output picture belongs */
        DecOutFrmFormat outputFormat; /**< Storage format of output picture. */
    } H264DecPicture;

    /*!\struct H264DecInfo_
     * \brief Decoded stream information
     *
     * A structure containing the decoded stream information, filled by
     * H264DecGetInfo()
     *
     * \typedef H264DecInfo
     * A typename for #H264DecInfo_.
     */
    typedef struct H264DecInfo_
    {
        u32 picWidth;        /**< decoded picture width in pixels */
        u32 picHeight;       /**< decoded picture height in pixels */
        u32 videoRange;      /**< samples' video range */
        u32 matrixCoefficients;
        H264CropParams cropParams;  /**< display cropping information */
        H264DecOutFormat outputFormat;  /**< format of the output picture */
        u32 sarWidth;        /**< sample aspect ratio */
        u32 sarHeight;       /**< sample aspect ratio */
        u32 monoChrome;      /**< is sequence monochrome */
        u32 interlacedSequence;      /**< is sequence interlaced */
        u32 dpbMode;         /**< DPB mode; frame, or field interlaced */
        u32 picBuffSize;     /**< number of picture buffers allocated and used by decoder */
        u32 multiBuffPpSize; /**< number of picture buffers needed in decoder+postprocessor multibuffer mode */
    } H264DecInfo;

    /*!\struct H264DecApiVersion_
     * \brief API Version information
     *
     * A structure containing the major and minor version number of the API.
     *
     * \typedef H264DecApiVersion
     * A typename for #H264DecApiVersion_.
     */
    typedef struct H264DecApiVersion_
    {
        u32 major;           /**< API major version */
        u32 minor;           /**< API minor version */
    } H264DecApiVersion;

    /*!\typedef H264DecBuild
     * \brief Build information
     *
     * A typename for #DecSwHwBuild_ containing the build information
     * of the decoder.
     */
    typedef struct DecSwHwBuild_  H264DecBuild;

    /*!\brief Stream consumed callback prototype
     *
     * This callback is invoked by the decoder to notify the application that
     * a stream buffer was fully processed and can be reused.
     *
     * \param pStream base address of a buffer that was set as input when
     *                calling H264DecDecode().
     * \param pUserData application provided pointer to some private data.
     *                  This is set at decoder initialization time.
     *
     * \sa H264DecMCInit();
     */
    typedef void H264DecMCStreamConsumed(u8 *pStream, void *pUserData);

    /*!\struct H264DecMCConfig_
     * \brief Multicore decoder init configuration
     *
     * \typedef H264DecMCConfig
     *  A typename for #H264DecMCConfig_.
     */
    typedef struct H264DecMCConfig_
    {
        /*! Disable internal display reordering.
         *  It can reduce the number of internally allocated picture buffers,
         *  but application has to do the display reordering. */
        u32 noOutputReordering;

        /*! Enable usage of extra frame buffers for smoother output.
         *  This can potentially double the number of internally allocated
         *  picture buffers.
         */
        u32 useDisplaySmoothing;

        DecDpbFlags dpbFlags;

        /*! Application provided callback for stream buffer processed. */
        H264DecMCStreamConsumed *streamConsumedCallback;
    } H264DecMCConfig;

/*------------------------------------------------------------------------------
    Prototypes of Decoder API functions
------------------------------------------------------------------------------*/

    /*!\brief Get API version information
     *
     * Return the version information of the SW API.
     * Static implementation, does not require a decoder instance.
     */
    H264DecApiVersion H264DecGetAPIVersion(void);

    /*!\brief Read SW/HW build information
     *
     * Returns the hardware and software build information of the decoder.
     * Static implementation, does not require a decoder instance.
     */
    H264DecBuild H264DecGetBuild(void);

    /*!\brief Create a single core decoder instance
     *
     * Single core decoder can decode both byte streams and NAL units.
     * FMO and ASO streams are supported also, but these will require an
     * internal switch to less hardware acceleration. For FMO and ASO stream
     * the entropy decoding is done in software.
     *
     * Every instance has to be released with H264DecRelease().
     *
     *\note Use H264DecMCInit() for creating an instance with multicore support.
     *
     */
    H264DecRet H264DecInit(H264DecInst *pDecInst, u32 noOutputReordering,
                           DecErrorHandling errorHandling,
                           u32 useDisplaySmoothing,
                           DecDpbFlags dpbFlags );

    /*!\brief Enable MVC decoding
     *
     * Use this to enable decoding of MVC streams. If not enabled, the decoder
     * can only decode the base view of an MVC streams.
     * MVC decoding has to be enabled  before any attempt to decode stream data
     * with H264DecDecode().
     *
     * \retval #H264DEC_OK for success.
     * \returns A negative return value signals an error.
     */
    H264DecRet H264DecSetMvc(H264DecInst pDecInst);

    /*!\brief Release a decoder instance
     *
     * H264DecRelease closes the decoder instance \c decInst and releases all
     * internally allocated resources.
     *
     * When connected with the Hantro HW post-processor, the post-processor
     * must be disconnected before releasing the decoder instance.
     * Refer to \ref PPDOC "PP API manual" for details.
     *
     * \param decInst instance to be released
     *
     * \retval #H264DEC_OK for success
     * \returns A negative return value signals an error.
     *
     * \sa H264DecInit()
     * \sa H264DecMCInit()
     *
     */
    void H264DecRelease(H264DecInst decInst);

    /*!\brief Decode data
     *
     * This function decodes one or more NAL units from the current stream.
     * The input buffer shall contain one of the following:
     *      - Exactly one NAL unit and nothing else.
     *      - One or more NAL units in byte stream format, as defined in
     *        Annex B of the standard [1].
     *
     *  The decoder automatically detects the format of the stream data and
     *  decodes NAL units until the whole buffer is processed or decoding of
     *  a picture is finished. The calling application may set \e picId field
     *  in the input structure to a unique value and use this to link a picture
     *  obtained from #H264DecNextPicture to a certain decoded stream data.
     *  This might be useful for example when the application obtains
     *  composition or display time for pictures by external means and needs
     *  to know when to display a picture returned by #H264DecNextPicture.
     *
     * \retval #H264DEC_STRM_PROCESSED
                All the data in the stream buffer processed. Stream buffer must
                be updated before calling #H264DecDecode again.
     * \retval #H264DEC_PIC_DECODED
                A new picture decoded. Single core mode has to call
                 #H264DecNextPicture to check if there are any pictures
                 available for displaying.
     * \retval #H264DEC_HDRS_RDY
     *          Headers decoded and activated. Stream header information is now
     *          readable with the function #H264DecGetInfo.
     * \retval #H264DEC_ADVANCED_TOOLS
     *          Current stream utilizes advanced coding tools, ASO and/or FMO.
     *          Decoder starts entropy decoding in software (much slower) and
     *          has to reinitialize.
     *
     * \returns A negative return value signals an error.
     */
    H264DecRet H264DecDecode(H264DecInst decInst,
                             const H264DecInput *pInput,
                             H264DecOutput *pOutput);

    /*!\brief Read next picture in display order
     *
     * \warning Do not use with multicore instances!
     *       Use instead H264DecMCNextPicture().
     *
     * \sa H264DecMCNextPicture()
     */
    H264DecRet H264DecNextPicture(H264DecInst decInst,
                                  H264DecPicture *pPicture, u32 endOfStream);

    /*!\brief Read decoded stream information
     *
     */
    H264DecRet H264DecGetInfo(H264DecInst decInst, H264DecInfo *pDecInfo);

    /*!\brief Read last decoded picture
     *
     * \warning Do not use with multicore instances!
     *       Use instead H264DecMCNextPicture().
     *
     * \sa H264DecMCNextPicture()
     */
    H264DecRet H264DecPeek(H264DecInst decInst, H264DecPicture *pPicture);

    /*!\brief Read number of HW cores available
     *
     * Multicore specific.
     *
     * Static implementation, does not require a decoder instance.
     *
     * \returns The number of available hardware decoding cores.
     */
    u32 H264DecMCGetCoreCount(void);

    /*!\brief Create a multicore decoder instance
     *
     * Multicore specific.
     *
     * Use this to initialize a new multicore decoder instance. In this mode
     * decoding will be done on a frame-by-frame bases. Only byte streams are
     * supported. Streams using FMO or ASO are not supported in multicore mode.
     *
     * Every instance has to be released with H264DecRelease().
     *
     * \param decInst pointer where the newly created instance will be stored.
     * \param pMCInitCfg initialization parameters, which  cannot be altered later.
     *
     * \retval #H264DEC_OK for success
     * \returns A negative return value signals an error.
     *
     * \sa H264DecRelease()
     */
    H264DecRet H264DecMCInit(H264DecInst *decInst,
                             H264DecMCConfig *pMCInitCfg);

    /*!\brief Decode data
     *
     * Multicore specific.
     *
     * The input buffer shall contain picture data for exactly one frame.
     * The buffer can also contain any number of other non picture data
     * units, like parameter sets, SEI messages, etc.
     * Only byte stream format is supported!
     *
     *  The calling application may set \e picId field
     *  in the input structure to a unique value and use this to link a picture
     *  obtained from #H264DecMCNextPicture to a certain decoded stream data.
     *  This might be useful for example when the application obtains
     *  composition or display time for pictures by external means and needs
     *  to know when to display a picture returned by #H264DecMCNextPicture.
     *
     * \retval #H264DEC_STRM_PROCESSED
                All the data in the stream buffer processed. Stream buffer must
                be updated before calling #H264DecMCDecode again.
     * \retval #H264DEC_PIC_DECODED
     *          A picture has been sent to hardware for decoding. Stream buffer
     *          cannot be reused until #H264DecMCStreamConsumed callback is
     *          issued.
     * \retval #H264DEC_HDRS_RDY
     *          Headers decoded and activated. Stream header information is now
     *          readable with the function #H264DecGetInfo.
     * \retval #H264DEC_ADVANCED_TOOLS
     *          Current stream utilizes advanced coding tools, ASO and/or FMO.
     *          This stream cannot be decoded in multicore mode.
     *
     * \returns A negative return value signals an error.
     */
    H264DecRet H264DecMCDecode(H264DecInst decInst,
                               const H264DecInput *pInput,
                               H264DecOutput *pOutput);

    /*!\brief Get next picture in display order
     *
     * Multicore specific.
     *
     * This function is used to get the decoded pictures out from the decoder.
     * The call will block until a picture is available for output or
     * an end-of-stream state is set in the decoder. Once processed, every
     * output picture has to be released back to decoder by application using
     * H264DecMCPictureConsumed(). Pass to this function the same data returned
     * in \c pPicture.
     *
     * \param decInst a multicore decoder instance.
     * \param pPicture pointer to a structure that will be filled with
     *        the output picture parameters.
     *
     * \retval #H264DEC_PIC_RDY to signal that a picture is available.
     * \retval #H264DEC_END_OF_STREAM to signal that the decoder is
     *          in end-of-stream state.
     * \returns A negative return value signals an error.
     *
     *
     * \sa H264DecMCPictureConsumed()
     * \sa H264DecMCEndOfStream()
     *
     */
    H264DecRet H264DecMCNextPicture(H264DecInst decInst,
                                    H264DecPicture *pPicture);

    /*!\brief Release picture buffer back to decoder
     *
     * Multicore specific.
     *
     * Use this function to return a picture back to the decoder once
     * consumed by application. Pictures are given to application by
     * H264DecMCNextPicture().
     *
     * \param decInst a multicore decoder instance.
     * \param pPicture pointer to data that identifies the picture buffer.
     *        Shall be the exact data returned by H264DecMCNextPicture().
     *
     * \retval #H264DEC_OK for success
     * \returns A negative return value signals an error.
     *
     * \sa  H264DecMCNextPicture()
     */
    H264DecRet H264DecMCPictureConsumed(H264DecInst decInst,
                                        const H264DecPicture *pPicture);

    /*!\brief Set end-of-stream state in multicore decoder
     *
     * Multicore specific.
     *
     * This function is used to set the decoder to end-of-stream state.
     * It must be called at the end of decoding so that all potentially
     * buffered pictures are flushed out and H264DecMCNextPicture()
     * is unblocked.
     * It is not safe to end the stream while #H264DecMCDecode is still
     * processing.
     *
     * This call will block until all cores have finished processing and all
     * output pictures are processed by application
     * (i.e. H264DecMCNextPicture() returns #H264DEC_END_OF_STREAM).
     *
     * \param decInst a multicore decoder instance.
     *
     * \retval #H264DEC_OK for success
     * \returns A negative return value signals an error.
     *
     * \sa  H264DecMCNextPicture()
     */
    H264DecRet H264DecMCEndOfStream(H264DecInst decInst);

    /*!\brief API internal tracing function prototype
     *
     * Traces all API entries and returns. This must be implemented by
     * the application using the decoder API.
     *
     * \param string Pointer to a NULL terminated char string.
     *
     */
    void H264DecTrace(const char *string);

    /*!\example h264dectrace.c
      * This is an example of how to implement H264DecTrace() in an application.
      *
      *!\example h264decmc_output_handling.c
      * This is an example of how to handle the decoder output in a
      * multi-threaded application.
      */

    /*! @}*/
#ifdef __cplusplus
}
#endif

#endif                       /* __H264DECAPI_H__ */
