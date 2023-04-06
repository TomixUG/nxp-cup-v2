/**
 * @author Hugues Angelis
 *
 * @section LICENSE
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * CMUCAM 5 - Pixy2
 *
 * Datasheet, FAQ and PC drivers :
 *
 * http://www.pixycam.com/
 */

#ifndef _PIXY2_
#define _PIXY2_

/**
 * Include : Mbed Library
 */
#include "mbed.h"

/**
 * Defines
 */
#define PIXY2_NCSHEADERSIZE 4
#define PIXY2_CSHEADERSIZE 4
#define PIXY2_SYNC 0xC1AE
#define PIXY2_CSSYNC 0xC1AF
#define PIXY2_REP_ACK 1
#define PIXY2_REP_ERROR 3
#define PIXY2_ASK_RESOL 12
#define PIXY2_REP_RESOL 13
#define PIXY2_ASK_VERS 14
#define PIXY2_REP_VERS 15
#define PIXY2_SET_BRIGHT 16
#define PIXY2_SET_SERVOS 18
#define PIXY2_SET_LED 20
#define PIXY2_SET_LAMP 22
#define PIXY2_ASK_FPS 24
#define PIXY2_REP_FPS 1
#define PIXY2_ASK_BLOC 32
#define PIXY2_REP_BLOC 33
#define PIXY2_ASK_LINE 48
#define PIXY2_REP_LINE 49
#define PIXY2_SET_MODE 54
#define PIXY2_SET_TURN 58
#define PIXY2_SET_VECTOR 56
#define PIXY2_SET_DEFTURN 60
#define PIXY2_SET_REVERSE 62
#define PIXY2_ASK_VIDEO 112
#define PIXY2_VECTOR 1
#define PIXY2_INTERSECTION 2
#define PIXY2_BARCODE 4
#define PIXY2_MAX_INT_LINE 6

/**************** ERRORS ****************/

/**
 *  \struct T_pixy2ErrorCode
 *  \brief Explicit error code list :
 *  \param PIXY2_OK                 : No error
 *  \param PIXY2_MISC_ERROR         : Generic error
 *  \param PIXY2_BUSY               : Pixy is busy (the response message is not yet fully arrived)
 *  \param PIXY2_BAD_CHECKSUM       : Checksum is wrong
 *  \param PIXY2_TIMEOUT            : Pixy2 is not talking
 *  \param PIXY2_BUTTON_OVERRIDE    : User is manualy operating the button of the Pixy2
 *  \param PIXY2_PROG_CHANGE        : Checksum is wrong
 *  \param PIXY2_TYPE_ERROR         : Unexpected message type
 *  @note More documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:general_api#error-codes
 */
typedef int T_pixy2ErrorCode;

#define PIXY2_OK 0
#define PIXY2_MISC_ERROR -1
#define PIXY2_BUSY -2
#define PIXY2_BAD_CHECKSUM -3
#define PIXY2_TIMEOUT -4
#define PIXY2_OVERRIDE -5
#define PIXY2_PROG_CHANGE -6
#define PIXY2_TYPE_ERROR -7

/**************** STATE MACHINE ****************/

typedef enum
{
    idle,
    messageSent,
    receivingHeader,
    receivingData,
    dataReceived
} T_Pixy2State;

/**************** UTILS ****************/

/**
 *  \struct Byte    ->      Short hand for unsigned char
 *  \struct sByte   ->      Short hand for char
 *  \struct Word    ->      Short hand for unsigned short
 *  \struct sWord   ->      Short hand for short
 *  \struct lWord   ->      Short hand for unsigned long
 *  \struct slWord  ->      Short hand for long
 */
typedef unsigned char Byte;
typedef char sByte;
typedef unsigned short Word;
typedef short sWord;
typedef unsigned long lWord;
typedef long slWord;

/**
 *  \union T_Word
 *  \brief  Structured type to switch from word to bytes
 *  \param  mot (Word)   : 16 bits word
 *  \param  octet (Byte) : 2 bytes that overlap mot (byte access)
 */
typedef union
{
    Word mot;
    Byte octet[2];
} T_Word;

/**
 *  \union T_lWord
 *  \brief  Structured type to switch from lword to word or bytes
 *  \param  motLong (lWord) : 32 bits word
 *  \param  mot (Word)      : 2 x 16 bits words that overlap motLong (word access)
 *  \param  octet (Byte)    : 4 bytes that overlap motLong (byte access)
 */
typedef union
{
    lWord motLong;
    Word mot[2];
    Byte octet[4];
} T_lWord;

/**************** HEADERS ****************/

/**
 *  \struct T_pixy2Header
 *  \brief  Structured type that match pixy2 header without checksum (send message)
 *  \param  pixSync   (Word) : 16 bits synchro word - could be 0xc1ae (PIXY2_SYNC) or 0xc1af (PIXY2_CSSYNC)
 *  \param  pixType   (Byte) : 8 bits message type identifier
 *  \param  pixLength (Byte) : 8 bits message payload length (payload doesn't include checksum)
 */
typedef struct
{
    Word pixSync;
    Byte pixType;
    Byte pixLength;
} T_pixy2Header;

/**
 *  \struct T_pixy2SendFrame
 *  \brief  Structured type that match frame definition for all kind of message to send to a pixy2
 *  \param  header (T_pixy2Header) : 4 bytes classical header starting with PIXY2_SYNC
 *  \param  data   (Byte)          : 5 bytes payload (to match all usage, not all byte must be used)
 */
typedef struct
{
    T_pixy2Header header;
    Byte data[5];
} T_pixy2SendFrame;

/**
 *  \union  T_pixy2SendBuffer
 *  \brief  Structured type to switch between structured type T_pixy2sendFrame and bytes
 *  \param  frame (T_pixy2SendFrame) : classical frame (header + payload) starting with PIXY2_SYNC
 *  \param  data  (Byte)             : 9 bytes that overlap frame (byte access)
 */
typedef union
{
    T_pixy2SendFrame frame;
    Byte data[9];
} T_pixy2SendBuffer;

/**
 *  \struct T_pixy2RcvHeader
 *  \brief  Structured type that match pixy2 header with checksum (received message)
 *  \param  pixSync   (Word) : 16 bits synchro word - could be 0xc1ae (PIXY2_SYNC) or 0xc1af (PIXY2_CSSYNC)
 *  \param  pixType   (Byte) : 8 bits message type identifier
 *  \param  pixLength (Byte) : 8 bits message payload length (payload doesn't include checksum)
 *  \param  pixSync   (Word) : 16 bits checksum (sum of all bytes of the payload)
 */
typedef struct
{
    Word pixSync;
    Byte pixType;
    Byte pixLength;
    Word pixChecksum;
} T_pixy2RcvHeader;

/**************** PAYLOADS ****************/

/**
 *  \struct T_pixy2ReturnCode
 *  \brief  Structured type that match pixy2 error/acknowledge/reply frame (type = 1 or 3) message payload
 *  \param  pixReturn (lWord) : 32 bits returned value
 */
typedef struct
{
    lWord pixReturn;
} T_pixy2ReturnCode;

/**
 *  \struct T_pixy2Version
 *  \brief  Structured type that match pixy2 version frame (type = 14/15) message payload
 *  \param  pixHWVersion    (Word)   : 16 bits hardWare Version of pixy2
 *  \param  pixFWVersionMaj (Byte)   : 8 bits upper part of firmware (before the dot)
 *  \param  pixFWVersionMin (Byte)   : 8 bits lower part of firmware (after the dot)
 *  \param  pixFWBuild      (Word)   : 16 bits firmware build information
 *  \param  pixHFString     (String) : 10 bytes user friendly pixy2 firmware type
 */
typedef struct
{
    Word pixHWVersion;
    Byte pixFWVersionMaj;
    Byte pixFWVersionMin;
    Word pixFWBuild;
    char pixHFString[10];
} T_pixy2Version;

/**
 *  \struct T_pixy2Resolution
 *  \brief  Structured type that match pixy2 resolution frame (type = 12/13) message payload
 *  \param  pixFrameWidth   (Word) : 16 bits width (in pixel) of an image
 *  \param  pixFrameHeight  (Word) : 16 bits height (in pixel) of an image
 */
typedef struct
{
    Word pixFrameWidth;
    Word pixFrameHeight;
} T_pixy2Resolution;

/**
 *  \struct T_pixy2Bloc
 *  \brief  Structured type that match pixy2 blocks frame (type = 32/33) message payload
 *  \param  pixSignature (Word)   : 16 bits signature or color code of the color bloc (signature are between 1 and 7, color code are composed of signature of 2, up to 5, tags so over 10)
 *  \param  pixX         (Word)   : 16 bits X (horizontal axis) position of color bloc center, relative to the left of the image (in pixels, between 0 and 315)
 *  \param  pixY         (Word)   : 16 bits Y (vertical axis) position of color bloc center, relative to the top of the image (in pixels, between 0 and 207)
 *  \param  pixWidth     (Word)   : 16 bits width (in pixels, between 0 and 316) of color bloc
 *  \param  pixHeight    (Word)   : 16 bits height (in pixels, between 0 and 208) of color bloc
 *  \param  pixAngle     (sWord)  : 16 bits angle (in degree, between -180.0 and +180.0) of a color code bloc
 *  \param  pixIndex     (Byte)   : 8 bits tracking identification of the color code bloc (set by pixy2 to ease a bloc position following program)
 *  \param  pixAge       (Byte)   : 8 bits age (in number of frame) of a bloc (doesn't wrap around).
 *  @note More info can be found here : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:ccc_api
 *  @note or here : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:color_connected_components
 */
typedef struct
{
    Word pixSignature;
    Word pixX;
    Word pixY;
    Word pixWidth;
    Word pixHeight;
    sWord pixAngle;
    Byte pixIndex;
    Byte pixAge;
} T_pixy2Bloc;

/**
 *  \struct T_pixy2Vector
 *  \brief  Structured type that match pixy2 vector definition - used in Line frame (type 48/49) - message payload
 *  \param  pixX0     (Byte)   : 8 bits X (horizontal, relative to the left of image) position of the tail of the vector (number between 0 and 78)
 *  \param  pixY0     (Byte)   : 8 bits Y (vertical, relative to the top of image) position of the tail of the vector (number between 0 and 51)
 *  \param  pixX1     (Byte)   : 8 bits X (horizontal, relative to the left of image) position of the head of the vector (number between 0 and 78)
 *  \param  pixY1     (Byte)   : 8 bits Y (vertical, relative to the top of image) position of the head of the vector (number between 0 and 51)
 *  \param  pixIndex  (Byte)   : 8 bits tracking identification of the vector (set by pixy2 to ease a vector identification in case of multiple vector in a line following program)
 *  \param  pixFlags  (Byte)   : 8 bits flag containing possible usefull informations (see notes)
 *  @note This structure is a feature of Line API, packed as a feature in a Line Frame, documented here : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:protocol_reference
 *  @note More info can be found here : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_api
 *  @note or here : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_tracking
 */
typedef struct
{
    Byte pixX0;
    Byte pixY0;
    Byte pixX1;
    Byte pixY1;
    Byte pixIndex;
    Byte pixFlags;
} T_pixy2Vector;

/**
 *  \struct T_pixy2InterLine
 *  \brief  Structured type that match pixy2 intersection line definition - used in Line frame (type 48/49) - message payload
 *  \param  pixIndex     (Byte)   : 8 bits tracking identification of the intersection line (set by pixy2 to ease a line following program)
 *  \param  pixReserved  (Byte)   : Not documented by manufacturer
 *  \param  pixAngle     (sWord)  : 16 bits angle (in degree, between -180.0 and +180.0) of the intersection line
 *  @note This structure is a sub feature of Line API, packed as a sub feature of intersection feature in a Line Frame, documented here : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:protocol_reference
 *  @note More info can be found here : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_api
 *  @note or here : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_tracking
 */
typedef struct
{
    Byte pixIndex;
    Byte pixReserved;
    sWord pixAngle;
} T_pixy2InterLine;

/**
 *  \struct T_pixy2Intersection
 *  \brief  Structured type that match pixy2 intersection definition - used in Line frame (type 48/49) - message payload
 *  \param  pixX         (Byte)   : X axis coordinate of the intersection (in pixel, between 0 and 78)
 *  \param  pixY         (Byte)   : Y axis coordinate of the intersection (in pixel, between 0 and 51)
 *  \param  pixN         (Byte)   : Number of lines connected to the intersection (between 3 and 5)
 *  \param  pixReserved  (Byte)   : Not documented by manufacturer
 *  @note This structure is a feature of Line API, packed as a feature in a Line Frame, documented here : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:protocol_reference
 *  @note More info can be found here : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_api
 *  @note or here : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_tracking
 */
typedef struct
{
    Byte pixX;
    Byte pixY;
    Byte pixN;
    Byte pixReserved;
    T_pixy2InterLine PixintLines[PIXY2_MAX_INT_LINE];
} T_pixy2Intersection;

/**
 *  \struct T_pixy2BarCode
 *  \brief  Structured type that match pixy2 barcode definition - used in Line frame (type 48/49) - message payload
 *  \param  pixX         (Byte)   : X axis coordinate of the barcode (in pixel, between 0 and 78)
 *  \param  pixY         (Byte)   : Y axis coordinate of the barcode (in pixel, between 0 and 51)
 *  \param  pixFlag      (Byte)   : Flag to indicate if barcode met filtering constraint
 *  \param  pixCode      (Byte)   : Indicate the numeric value associated with the barcode (between 0 and 15)
 *  @note This structure is a feature of Line API, packed as a feature in a Line Frame, documented here : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:protocol_reference
 *  @note More info can be found here : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_api
 *  @note or here : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_tracking
 */
typedef struct
{
    Byte pixX;
    Byte pixY;
    Byte pixFlag;
    Byte pixCode;
} T_pixy2BarCode;

/**
 *  \struct T_pixy2LineFeature
 *  \brief  Structured type that match pixy2 feature header for Line API - used in Line frame (type 48/49) - message payload
 *  \param  pixType      (Byte)   : Type of the feature (can be 1 -> Vector, 2 -> Intersection or 4 -> Barcode)
 *  \param  pixLength    (Byte)   : Number of Bytes for this feature
 *  @note This structure is the header of features of Line API in Line Frames. Ther can be up to 4 features in a frame. Documented here : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:protocol_reference
 *  @note More info can be found here : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_api
 *  @note or here : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_tracking
 */
typedef struct
{
    Byte fType;
    Byte fLength;
} T_pixy2LineFeature;

/**
 *  \struct T_pixy2Pixel
 *  \brief  Structured type that match pixy2 video API - used in Video frame (type 112/1) - message payload
 *  \param  pixBlue      (Byte)   : Blue RGB value of the average blue component of the 5x5 pixels square centered on X param passes to the function (value between 0 and 255)
 *  \param  pixGreen     (Byte)   : Green RGB value of the average blue component of the 5x5 pixels square centered on X param passes to the function (value between 0 and 255)
 *  \param  pixRed      (Byte)   : Red RGB value of the average blue component of the 5x5 pixels square centered on X param passes to the function (value between 0 and 255)
 *  @note More info can be found here : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:video_api
 *  @note or here : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:video
 */
typedef struct
{
    Byte pixBlue;
    Byte pixGreen;
    Byte pixRed;
} T_pixy2Pixel;

/**
 * Pixy2 : CMU CAM 5 - Smart camera
 * More informations at http://www.pixycam.com/
 */
class PIXY2
{

protected:
    Serial *_Pixy2;

public:
    /**
     * Constructor of pixy2 UART object.
     *
     * @param tx : the Mbed pin used as TX
     * @param rx : the Mbed pin used as RX
     * @param debit : the bitrate of the serial (max value is 230400 b/s)
     */
    PIXY2(PinName tx, PinName rx, int debit = 230400);

    /**
     * Destructor of pixy2 UART object.
     */
    ~PIXY2();

    // Fonctions publiques

    /**
     * Queries and receives the firmware and hardware version of Pixy2, which is put in the version member variable.
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:general_api
     * @param version (T_pixy2Version - passed by reference) : pointer to the version data structure
     * @return T_pixy2ErrorCode : error code.
     */
    T_pixy2ErrorCode pixy2_getVersion(T_pixy2Version *version);

    /**
     * Gets the width and height of the frames used by the current program.
     * After calling this function, the width and height can be found in the frameWidth and frameHeight member variables.
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:general_api
     * @param resolution (T_pixy2Resolution - passed by reference) : pointer to the resolution data structure
     * @return T_pixy2ErrorCode : error code.
     */
    T_pixy2ErrorCode pixy2_getResolution(T_pixy2Resolution *resolution);

    /**
     * Sets the relative exposure level of Pixy2's image sensor.
     * Higher values result in a brighter (more exposed) image.
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:general_api
     * @param brightness (Byte - passed by value) : brightness level
     * @return T_pixy2ErrorCode : error code.
     */
    T_pixy2ErrorCode pixy2_setCameraBrightness(Byte brightness);

    /**
     * Sets the servo positions of servos plugged into Pixy2's two RC servo connectors.
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:general_api
     * @param s0 (Word - passed by value) : value between 0 and 511
     * @param s1 (Word - passed by value) : value between 0 and 511
     * @return T_pixy2ErrorCode : error code.
     */
    T_pixy2ErrorCode pixy2_setServos(Word s0, Word s1);

    /**
     * Sets Pixy2's RGB LED value. The three arguments sets the brightness of the red, green and blue sections of the LED.
     * It will override Pixy2's own setting of the RGB LED.
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:general_api
     * @param red (Byte - passed by value) : Red component value (between 0 and 255)
     * @param green (Byte - passed by value) : Green component value (between 0 and 255)
     * @param blue (Byte - passed by value) : Blue component value (between 0 and 255)
     * @return T_pixy2ErrorCode : error code.
     */
    T_pixy2ErrorCode pixy2_setLED(Byte red, Byte green, Byte blue);

    /**
     * Sets on/off Pixy2's integrated light source.
     * The upper argument controls the two white LEDs along the top edge of Pixy2's PCB. The lower argument sets the RGB LED, causing it to turn on all three color channels at full brightness, resulting in white light.
     * It will override Pixy2's own setting of the RGB LED.
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:general_api
     * @param upper (Byte - passed by value) : switch on or off the upper lamps (boolean : zero or non-zero)
     * @param lower (Byte - passed by value) : switch on or off the lower lamp (boolean : zero or non-zero)
     * @return T_pixy2ErrorCode : error code.
     */
    T_pixy2ErrorCode pixy2_setLamp(Byte upper, Byte lower);

    /**
     * Gets Pixy2's framerate.
     * The framerate can range between 2 and 62 frames per second depending on the amount of light in the environment and the min frames per second setting in the Camera configuration tab.
     * This function can also serve as a simple indicator of the amount of light in the environment. That is, low framerates necessarily imply lower lighting levels
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:general_api
     * @param framerate (T_pixy2ReturnCode - passed by reference) : number of frame per second (between 2 and 62)
     * @return T_pixy2ErrorCode : error code.
     */
    T_pixy2ErrorCode pixy2_getFPS(T_pixy2ReturnCode *framerate);

    /**
     * Gets all detected color blocks in the most recent frame.
     * The new data is then available in the blocks member variable. The returned blocks are sorted by area, with the largest blocks appearing first in the blocks array.
     * @note General description : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:color_connected_components
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:pixy2_full_api#plugin_include__wiki__v2__ccc_api
     * @param sigmap (Byte - passed by value) : signature filtering
     * @param maxBloc (Byte - passed by value) : maximum number of blocks to return
     * @return T_pixy2ErrorCode : error code.
     * @note There are 7 different signatures definition (sig1 to sig7). Color codes are made of a combination of signature and can be filtered as well.
     * @note Filtering is based on ORing codes : 1 for sig1, 2 for sig2, 4 for sig3, 8 for sig4, 16 for sig5, 32 for sig6, 64 sor sig7 and 128 for color code.
     * @note So sigmap = 255 means accept all and sigmap = 0 means reject all. For example filtering to get only sig1 and sig5 means using sigmap = 17 (16 + 1).
     */
    T_pixy2ErrorCode pixy2_getBlocks(Byte sigmap, Byte maxBloc);

    /**
     * Gets the latest main features of Line tracking in the most recent frame.
     * The results are returned in the variables vectors, intersections, and barcodes, respectively.
     * The main feature is the feature that is the most likely to be relevant for line traking.
     * In case of multiple vector (for example 2 lines unconnected), the function will return only the vector of the line you are the most likely to follow.
     * @note General description : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_tracking
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:pixy2_full_api#plugin_include__wiki__v2__line_api
     * @param feature (Byte - passed by value) : feature filtering
     * @return T_pixy2ErrorCode : error code.
     * @note There are 3 possible features (vectors, intersections and barcodes).
     * @note Filtering is based on ORing codes : 1 for vectors, 2 for intersections, 4 for barcodes.
     * @note So 7 = accept all, 0 = reject all. For example filtering to get only vectors and barcode means using feature = 5 (1 + 4).
     */
    T_pixy2ErrorCode pixy2_getMainFeature(Byte features);

    /**
     * Gets all the latest features of Line tracking in the most recent frame.
     * The results are returned in the variables vectors[], intersections[], and barcodes[], respectively.
     * @note General description : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_tracking
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:pixy2_full_api#plugin_include__wiki__v2__line_api
     * @param feature (Byte - passed by value) : feature filtering
     * @return T_pixy2ErrorCode : error code (if negative) or ORing of feature detected (if positive).
     * @note There are 3 possible features (vectors, intersections and barcodes).
     * @note Filtering or detected feature are based on ORing codes : 1 for vectors, 2 for intersections, 4 for barcodes.
     * @note So for filtering : 7 = accept all, 0 = reject all. For example filtering to get only vectors and barcode means using feature = 5 (1 + 4).
     * @note So for return value : 1 means only vector(s) detected, 2 means only intersection(s) detected, 3 vector(s) and intersection(s) detected and so on.
     */
    T_pixy2ErrorCode pixy2_getAllFeature(Byte features);

    /**
     * Sets various modes in the line tracking algorithm.
     * @note General description : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_tracking
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:pixy2_full_api#plugin_include__wiki__v2__line_api
     * @param mode (Byte - passed by value) : ORing of required feature
     * @return T_pixy2ErrorCode : error code.
     * @note There are 3 possible features :
     * @note * LINE_MODE_TURN_DELAYED : Normally, the line tracking algorithm will choose the straightest path (branch) when encountering an intersection. Setting LINE_MODE_TURN_DELAYED will prevent the line tracking algorithm from choosing the path automatically. This is useful if your program doesn't know beforehand which path it wants to take at the next intersection.
     * @note * LINE_MODE_MANUAL_SELECT_VECTOR : Normally, the line tracking algorithm will choose what it thinks is the best Vector line automatically. Setting LINE_MODE_MANUAL_SELECT_VECTOR will prevent the line tracking algorithm from choosing the Vector automatically. Instead, your program will need to set the Vector by calling setVector().
     * @note * LINE_MODE_WHITE_LINE : Normally, the line tracking algorithm will try to find dark lines on a light background (black lines). Setting LINE_MODE_WHITE_LINE will instruct the line tracking algorithm to look for light lines on a dark background (white lines).
     */
    T_pixy2ErrorCode pixy2_setMode(Byte mode);

    /**
     * Tells the line tracking algorithm which path it should take at the next intersection.
     * @note General description : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_tracking
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:pixy2_full_api#plugin_include__wiki__v2__line_api
     * @param angle (sWord - passed by value) : angle closest to the next turn (in degree, between -180 and 180)
     * @return T_pixy2ErrorCode : error code.
     * @note Turn angles are specified in degrees, with 0 being straight ahead, left being 90 and right being -90, although any valid angle value can be used.
     * @note setNextTurn() will remember the turn angle you give it, and execute it at the next intersection. The line tracking algorithm will then go back to the default turn angle for subsequent intersections.
     * @note Upon encountering an intersection, the line tracking algorithm will find the path in the intersection that matches the turn angle most closely.
     */
    T_pixy2ErrorCode pixy2_setNextTurn(sWord angle);

    /**
     * Tells the line tracking algorithm which path to choose by default upon encountering an intersection.
     * @note General description : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_tracking
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:pixy2_full_api#plugin_include__wiki__v2__line_api
     * @param angle (sWord - passed by value) : angle closest to the default turn (in degree, between -180 and 180)
     * @return T_pixy2ErrorCode : error code.
     * @note Turn angles are specified in degrees, with 0 being straight ahead, left being 90 and right being -90, although any valid angle value can be used.
     * @note The line tracking algorithm will find the path in the intersection that matches the default turn angle most closely.
     * @note A call to setNextTurn() will supercede the default turn angle for the next intersection.
     */
    T_pixy2ErrorCode pixy2_setDefaultTurn(sWord angle);

    /**
     * Tells witch vector the tracking algorithm must choose as default route upon encountering an intersection.
     * @note General description : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_tracking
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:pixy2_full_api#plugin_include__wiki__v2__line_api
     * @param index (Byte - passed by value) : index of the line to follow
     * @return T_pixy2ErrorCode : error code.
     * @note If the LINE_MODE_MANUAL_SELECT_VECTOR mode bit is set, the line tracking algorithm will no longer choose the Vector automatically. Instead, setVector() will set the Vector by providing the index of the line.
     */
    T_pixy2ErrorCode pixy2_setVector(Byte vectorIndex);

    /**
     * Reverse the head and the tail of vectors
     * @note General description : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_tracking
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:pixy2_full_api#plugin_include__wiki__v2__line_api
     * @return T_pixy2ErrorCode : error code.
     * @note The Vector has a direction. It normally points up, from the bottom of the camera frame to the top of the camera frame for a forward-moving robot. Calling reverseVector() will invert the vector. This will typically cause your robot to back-up and change directions.
     */
    T_pixy2ErrorCode pixy2_ReverseVector(void);

    /**
     * Returns the average RGB components of a square of 5x5 pixels centered on the given pixel coordinate
     * @note General description : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:video
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:video_api
     * @param x (Word - passed by value) : X coordinate of the center of de 5x5 pixels square (in pixel, between 0 and 315)
     * @param y (Word - passed by value) : Y coordinate of the center of de 5x5 pixels square (in pixel, between 0 and 207)
     * @param saturate (Byte - passed by value) : scale the 3 RGB components so that the highest one of the 3 RGB components is set to 255 (boolean : zero or non-zero)
     * @param pixel (T_pixy2Pixel - passed by reference) : RGB pixel.
     * @return T_pixy2ErrorCode : error code.
     */
    T_pixy2ErrorCode pixy2_getRGB(Word x, Word y, Byte saturate, T_pixy2Pixel *pixel);

    // Variables globales Publiques
    /**
     * @var Pixy2_numBlocks (Byte) number of color blocks in Pixy2_blocks
     * @var Pixy2_blocks[] (T_pixy2Bloc) color blocks detected in the last frame
     * @var Pixy2_numVectors (Byte) number of vectors in Pixy2_vectors
     * @var Pixy2_vectors[] (T_pixy2Vector) vectors detected in the last frame
     * @var Pixy2_numIntersections (Byte) number of intersections in Pixy2_intersections
     * @var Pixy2_intersections[] (T_pixy2Intersection) intersections detected in the last frame
     * @var Pixy2_intersLine[] (T_pixy2IntLines) lines detected in the last frame
     * @var Pixy2_numVectors (Byte) number of vectors in Pixy2_blocks
     * @var Pixy2_vectors[] (T_pixy2Vector) vectors detected in the last frame
     */
    Byte Pixy2_numBlocks;
    T_pixy2Bloc *Pixy2_blocks;
    Byte Pixy2_numVectors;
    T_pixy2Vector *Pixy2_vectors;
    Byte Pixy2_numIntersections;
    T_pixy2Intersection *Pixy2_intersections;
    Byte Pixy2_numBarcodes;
    T_pixy2BarCode *Pixy2_barcodes;

private:
    // Variables globales Privées
    /**
     * @var etat (T_Pixy2State) state of the pixy2 cam (idle = No action, messageSent = Query or Set message sent, receivingHeader = Camera is respondig to the query/set, receivingData = Header received, dataReceived = All data has been recovered)
     * @var Pixy2_buffer (Array of Byte) bytes received from camera
     * @var wPointer (Byte) write pointer, pointing the next free cell of the array of received bytes
     * @var hPointer (Byte) header pointer, pointing on the begining of the header field in the array of received bytes
     * @var dPointer (Byte) data pointer, pointing on the begining of the data field in the array of received bytes
     * @var dataSize (Byte) number of bytes in the data field
     * @var frameContainChecksum (Byte) indicate if the received frame contains a checksum
     */
    T_Pixy2State etat;
    Byte *Pixy2_buffer;
    Byte wPointer, hPointer, dPointer, dataSize;
    Byte frameContainChecksum;

    // Fonctions privées

    /**
     * Queries the firmware and hardware version of Pixy2.
     * This function is part of the pixy2_getVersion function who treat the reply to this query.
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:general_api
     * @return T_pixy2ErrorCode : error code.
     */
    T_pixy2ErrorCode pixy2_sndGetVersion(void);

    /**
     * Queries width and height of the frames used by the current program.
     * This function is part of the pixy2_getResolution function who treat the reply to this query.
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:general_api
     * @return T_pixy2ErrorCode : error code.
     */
    T_pixy2ErrorCode pixy2_sndGetResolution(void);

    /**
     * Sends the relative exposure level of Pixy2's image sensor.
     * Higher values result in a brighter (more exposed) image.
     * This function is part of the pixy2_setCameraBrightness function who treat the acknowledge of this frame.
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:general_api
     * @param brightness (Byte - passed by value) : brightness level
     * @return T_pixy2ErrorCode : error code.
     */
    T_pixy2ErrorCode pixy2_sndSetCameraBrightness(Byte brightness);

    /**
     * Sends the servo positions of servos plugged into Pixy2's two RC servo connectors.
     * This function is part of the pixy2_setServo function who treat the acknowledge of this frame.
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:general_api
     * @param s0 (Word - passed by value) : value between 0 and 511
     * @param s1 (Word - passed by value) : value between 0 and 511
     * @return T_pixy2ErrorCode : error code.
     */
    T_pixy2ErrorCode pixy2_sndSetServo(Word s0, Word s1);

    /**
     * Sends Pixy2's RGB LED value. The three arguments sets the brightness of the red, green and blue sections of the LED.
     * It will override Pixy2's own setting of the RGB LED.
     * This function is part of the pixy2_setLED function who treat the acknowledge of this frame.
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:general_api
     * @param red (Byte - passed by value) : Red component value (between 0 and 255)
     * @param green (Byte - passed by value) : Green component value (between 0 and 255)
     * @param blue (Byte - passed by value) : Blue component value (between 0 and 255)
     * @return T_pixy2ErrorCode : error code.
     */
    T_pixy2ErrorCode pixy2_sndSetLED(Byte red, Byte green, Byte blue);

    /**
     * Sends command that turns on/off Pixy2's integrated light source.
     * The upper argument controls the two white LEDs along the top edge of Pixy2's PCB. The lower argument sets the RGB LED, causing it to turn on all three color channels at full brightness, resulting in white light.
     * It will override Pixy2's own setting of the RGB LED.
     * This function is part of the pixy2_setLamp function who treat the acknowledge of this frame.
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:general_api
     * @param upper (Byte - passed by value) : switch on or off the upper lamps (boolean : zero or non-zero)
     * @param lower (Byte - passed by value) : switch on or off the lower lamp (boolean : zero or non-zero)
     * @return T_pixy2ErrorCode : error code.
     */
    T_pixy2ErrorCode pixy2_sndSetLamp(Byte upper, Byte lower);

    /**
     * Queries Pixy2's framerate.
     * This function is part of the pixy2_getFPS function who treat the reply to this query.
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:general_api
     * @return T_pixy2ErrorCode : error code.
     */
    T_pixy2ErrorCode pixy2_sndGetFPS(void);

    /**
     * Queries all detected color blocks in the most recent frame.
     * This function is part of the pixy2_getBlocks function who treat the reply to this query.
     * @note General description : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:color_connected_components
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:pixy2_full_api#plugin_include__wiki__v2__ccc_api
     * @param sigmap (Byte - passed by value) : signature filtering
     * @param maxBloc (Byte - passed by value) : maximum number of blocks to return
     * @return T_pixy2ErrorCode : error code.
     * @note There are 7 different signatures definition (sig1 to sig7). Color codes are made of a combination of signature and can be filtered as well.
     * @note Filtering is based on ORing codes : 1 for sig1, 2 for sig2, 4 for sig3, 8 for sig4, 16 for sig5, 32 for sig6, 64 sor sig7 and 128 for color code.
     * @note So sigmap = 255 means accept all and sigmap = 0 means reject all. For example filtering to get only sig1 and sig5 means using sigmap = 17 (16 + 1).
     */
    T_pixy2ErrorCode pixy2_sndGetBlocks(Byte sigmap, Byte maxBloc);

    /**
     * Queries the latest features of Line tracking in the most recent frame.
     * The results are returned in the variables vectors, intersections, and barcodes, respectively.
     * This function is part of the pixy2_getMainFeature or pixy2_getAllFeature function who treat the reply to this query.
     * @note General description : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_tracking
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:pixy2_full_api#plugin_include__wiki__v2__line_api
     * @param type (Byte - passed by value) : select between Main or All features (0 for main feature only, 1 for all features)
     * @param feature (Byte - passed by value) : feature filtering
     * @return T_pixy2ErrorCode : error code (if negative) or ORing of feature detected (if positive).
     * @note There are 3 possible features (vectors, intersections and barcodes).
     * @note Filtering or detected feature are based on ORing codes : 1 for vectors, 2 for intersections, 4 for barcodes.
     * @note So for filtering : 7 = accept all, 0 = reject all. For example filtering to get only vectors and barcode means using feature = 5 (1 + 4).
     * @note So for return value : 1 means only vector(s) detected, 2 means only intersection(s) detected, 3 vector(s) and intersection(s) detected and so on.
     */
    T_pixy2ErrorCode pixy2_sndGetLineFeature(Byte type, Byte feature);

    /**
     * Sets various modes in the line tracking algorithm.
     * This function is part of the pixy2_setMode function who treat the acknowledge of this frame.
     * @note General description : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_tracking
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:pixy2_full_api#plugin_include__wiki__v2__line_api
     * @param mode (Byte - passed by value) : ORing of required feature
     * @return T_pixy2ErrorCode : error code.
     * @note There are 3 possible features :
     * @note * LINE_MODE_TURN_DELAYED : Normally, the line tracking algorithm will choose the straightest path (branch) when encountering an intersection. Setting LINE_MODE_TURN_DELAYED will prevent the line tracking algorithm from choosing the path automatically. This is useful if your program doesn't know beforehand which path it wants to take at the next intersection.
     * @note * LINE_MODE_MANUAL_SELECT_VECTOR : Normally, the line tracking algorithm will choose what it thinks is the best Vector line automatically. Setting LINE_MODE_MANUAL_SELECT_VECTOR will prevent the line tracking algorithm from choosing the Vector automatically. Instead, your program will need to set the Vector by calling setVector().
     * @note * LINE_MODE_WHITE_LINE : Normally, the line tracking algorithm will try to find dark lines on a light background (black lines). Setting LINE_MODE_WHITE_LINE will instruct the line tracking algorithm to look for light lines on a dark background (white lines).
     */
    T_pixy2ErrorCode pixy2_sndSetMode(Byte mode);

    /**
     * Tells the line tracking algorithm which path it should take at the next intersection.
     * This function is part of the pixy2_setNextTurn function who treat the acknowledge of this frame.
     * @note General description : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_tracking
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:pixy2_full_api#plugin_include__wiki__v2__line_api
     * @param angle (sWord - passed by value) : angle closest to the next turn (in degree, between -180 and 180)
     * @return T_pixy2ErrorCode : error code.
     * @note Turn angles are specified in degrees, with 0 being straight ahead, left being 90 and right being -90, although any valid angle value can be used.
     * @note setNextTurn() will remember the turn angle you give it, and execute it at the next intersection. The line tracking algorithm will then go back to the default turn angle for subsequent intersections.
     * @note Upon encountering an intersection, the line tracking algorithm will find the path in the intersection that matches the turn angle most closely.
     */
    T_pixy2ErrorCode pixy2_sndSetNextTurn(Word angle);

    /**
     * Tells the line tracking algorithm which path to choose by default upon encountering an intersection.
     * This function is part of the pixy2_setDefaultTurn function who treat the acknowledge of this frame.
     * @note General description : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_tracking
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:pixy2_full_api#plugin_include__wiki__v2__line_api
     * @param angle (sWord - passed by value) : angle closest to the default turn (in degree, between -180 and 180)
     * @return T_pixy2ErrorCode : error code.
     * @note Turn angles are specified in degrees, with 0 being straight ahead, left being 90 and right being -90, although any valid angle value can be used.
     * @note The line tracking algorithm will find the path in the intersection that matches the default turn angle most closely.
     * @note A call to setNextTurn() will supercede the default turn angle for the next intersection.
     */
    T_pixy2ErrorCode pixy2_sndSetDefaultTurn(Word angle);

    /**
     * Tells witch vector the tracking algorithm must choose as default route upon encountering an intersection.
     * This function is part of the pixy2_setVector function who treat the acknowledge of this frame.
     * @note General description : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_tracking
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:pixy2_full_api#plugin_include__wiki__v2__line_api
     * @param vectorindex (Byte - passed by value) : index of the vector to follow
     * @return T_pixy2ErrorCode : error code.
     * @note If the LINE_MODE_MANUAL_SELECT_VECTOR mode bit is set, the line tracking algorithm will no longer choose the Vector automatically. Instead, setVector() will set the Vector by providing the index of the line.
     */
    T_pixy2ErrorCode pixy2_sndSetVector(Byte vectorIndex);

    /**
     * Reverse the head and the tail of vectors
     * This function is part of the pixy2_reverseVector function who treat the acknowledge of this frame.
     * @note General description : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_tracking
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:pixy2_full_api#plugin_include__wiki__v2__line_api
     * @return T_pixy2ErrorCode : error code.
     * @note The Vector has a direction. It normally points up, from the bottom of the camera frame to the top of the camera frame for a forward-moving robot. Calling reverseVector() will invert the vector. This will typically cause your robot to back-up and change directions.
     */
    T_pixy2ErrorCode pixy2_sndReverseVector(void);

    /**
     * Queries the average RGB components of a square of 5x5 pixels centered on the given pixel coordinate
     * This function is part of the pixy2_getRGB function who treat the reply to this query.
     * @note General description : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:video
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:video_api
     * @param x (Word - passed by value) : X coordinate of the center of de 5x5 pixels square (in pixel, between 0 and 315)
     * @param y (Word - passed by value) : Y coordinate of the center of de 5x5 pixels square (in pixel, between 0 and 207)
     * @param saturate (Byte - passed by value) : scale the 3 RGB components so that the highest one of the 3 RGB components is set to 255 (boolean : zero or non-zero)
     * @param pixel (T_pixy2Pixel - passed by reference) : RGB pixel.
     * @return T_pixy2ErrorCode : error code.
     */
    T_pixy2ErrorCode pixy2_sndGetRGB(Word x, Word y, Byte saturate);

    /**
     * Gets all the latest features of Line tracking in the most recent frame.
     * The results are returned in the variables vectors, intersections, and barcodes, respectively.
     * @note General description : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:line_tracking
     * @note Frame Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:porting_guide
     * @note Function Documentation : https://docs.pixycam.com/wiki/doku.php?id=wiki:v2:pixy2_full_api#plugin_include__wiki__v2__line_api
     * @param feature (Byte - passed by value) : feature filtering
     * @return T_pixy2ErrorCode : error code.
     * @note There are 3 possible features (vectors, intersections and barcodes).
     * @note Filtering is based on ORing codes : 1 for vectors, 2 for intersections, 4 for barcodes.
     * @note So 7 = accept all, 0 = reject all. For example filtering to get only vectors and barcode means using feature = 5 (1 + 4).
     */
    T_pixy2ErrorCode pixy2_getFeatures(void);

    void pixy2_getByte();
    T_pixy2ErrorCode pixy2_validateChecksum(Byte *tab);
}; // End Class

#endif