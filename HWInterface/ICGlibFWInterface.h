
/*!

        \file                           ICICGlibFWInterface.h
        \brief                          ICICGlibFWInterface init/config of the Glib and its Cbc's
        \author                         G. Auzinger, K. Uchida
        \version            1.0
        \date                           25.02.2016
        Support :                       mail to : georg.auzinger@SPAMNOT.cern.ch

 */

#ifndef __ICGLIBFWINTERFACE_H__
#define __ICGLIBFWINTERFACE_H__

#include <string>
#include <map>
#include <vector>
#include <limits.h>
#include <stdint.h>
#include "BeBoardFWInterface.h"
#include "../HWDescription/Module.h"
#include "../Utils/Visitor.h"

#define RESET 0x1
#define START 0x2
#define STOP  0x4
#define CTR_RESET 0x800
#define HARD_RESET 0x1
#define FAST_RESET 0x2
#define I2C_REFRESH 0x4
#define TEST_PULSE 0x8
#define L1A        0x010

using namespace Ph2_HwDescription;

/*!
 * \namespace Ph2_HwInterface
 * \brief Namespace regrouping all the interfaces to the hardware
 */
namespace Ph2_HwInterface {
    class FpgaConfig;
    /*!
     * \class ICGlibFWInterface
     * \brief init/config of the Glib and its Cbc's
     */
    class ICGlibFWInterface : public BeBoardFWInterface
    {

      private:
        Data* fData; /*!< Data read storage*/

        struct timeval fStartVeto;
        FpgaConfig* fpgaConfig;
        FileHandler* fFileHandler ;
        uint32_t fBroadcastCbcId;
        uint32_t fReplyBufferSize;
        uint32_t fNEventsperAcquistion;
        uint32_t fDataSizeperEvent32;
        uint32_t fFMCId;

        const uint32_t SINGLE_I2C_WAIT = 70; //usec for 1MHz I2C
        //  const uint32_t SINGLE_I2C_WAIT = 700; //usec for 100 kHz I2C


      public:
        /*!
         * \brief Constructor of the ICGlibFWInterface class
         * \param puHalConfigFileName : path of the uHal Config File
         * \param pBoardId
         */
        ICGlibFWInterface ( const char* puHalConfigFileName, uint32_t pBoardId );
        ICGlibFWInterface ( const char* puHalConfigFileName, uint32_t pBoardId, FileHandler* pFileHandler );
        /*!
        * \brief Constructor of the ICGlibFWInterface class
        * \param pId : ID string
        * \param pUri: URI string
        * \param pAddressTable: address tabel string
        */
        ICGlibFWInterface ( const char* pId, const char* pUri, const char* pAddressTable );
        ICGlibFWInterface ( const char* pId, const char* pUri, const char* pAddressTable, FileHandler* pFileHandler );

        /*!
         * \brief Destructor of the ICGlibFWInterface class
         */
        ~ICGlibFWInterface()
        {
            if (fData) delete fData;
        }

        ///////////////////////////////////////////////////////
        //      Glib Methods                                //
        /////////////////////////////////////////////////////

        /*! \brief Read a block of a given size
         * \param pRegNode Param Node name
         * \param pBlocksize Number of 32-bit words to read
         * \return Vector of validated 32-bit values
         */
        std::vector<uint32_t> ReadBlockRegValue ( const std::string& pRegNode, const uint32_t& pBlocksize ) override;

        bool WriteBlockReg ( const std::string& pRegNode, const std::vector< uint32_t >& pValues ) override;
        /*!
         * \brief Get the FW info
         */
        void getBoardInfo();

        BoardType getBoardType() const
        {
            return BoardType::ICGLIB;
        }
        /*!
         * \brief Configure the board with its Config File
         * \param pBoard
         */
        void ConfigureBoard ( const BeBoard* pBoard ) override;
        /*!
         * \brief Detect the right FE Id to write the right registers (not working with the latest Firmware)
         */
        void SelectFEId();
        /*!
         * \brief Start a DAQ
         */
        void Start() override;
        /*!
         * \brief Stop a DAQ
         */
        void Stop() override;
        /*!
         * \brief Pause a DAQ
         */
        void Pause() override;
        /*!
         * \brief Unpause a DAQ
         */
        void Resume() override;
        /*!
         * \brief Read data from DAQ
         * \param pBreakTrigger : if true, enable the break trigger
         * \return fNpackets: the number of packets read
         */
        uint32_t ReadData ( BeBoard* pBoard, bool pBreakTrigger ) override;
        /*!
         * \brief Read data for pNEvents
         * \param pBoard : the pointer to the BeBoard
         * \param pNEvents :  the 1 indexed number of Events to read - this will set the packet size to this value -1
         */
        void ReadNEvents (BeBoard* pBoard, uint32_t pNEvents);
        /*!
         * \brief Get next event from data buffer
         * \return Next event
         */
        const Event* GetNextEvent ( const BeBoard* pBoard ) const override
        {
            return fData->GetNextEvent ( pBoard );
        }
        const Event* GetEvent ( const BeBoard* pBoard, int i ) const override
        {
            return fData->GetEvent ( pBoard, i );
        }
        const std::vector<Event*>& GetEvents ( const BeBoard* pBoard ) const override
        {
            return fData->GetEvents ( pBoard );
        }

      private:

        //I2C command sending implementation
        bool WriteI2C (  std::vector<uint32_t>& pVecSend, std::vector<uint32_t>& pReplies, bool pWriteRead, bool pBroadcast );
        bool ReadI2C (  uint32_t pNReplies, std::vector<uint32_t>& pReplies);

        //binary predicate for comparing sent I2C commands with replies using std::mismatch
        static bool cmd_reply_comp (const uint32_t& cWord1, const uint32_t& cWord2);
        static bool cmd_reply_ack (const uint32_t& cWord1, const uint32_t& cWord2);

        //template to copy every nth element out of a vector to another vector
        template<class in_it, class out_it>
        out_it copy_every_n ( in_it b, in_it e, out_it r, size_t n)
        {
            for (size_t i = std::distance (b, e) / n; i--; std::advance (b, n) )
                *r++ = *b;

            return r;
        }

        //method to split a vector in vectors that contain elements from even and odd indices
        void splitVectorEvenOdd (std::vector<uint32_t> pInputVector, std::vector<uint32_t>& pEvenVector, std::vector<uint32_t>& pOddVector)
        {
            bool ctoggle = false;
            std::partition_copy (pInputVector.begin(),
                                 pInputVector.end(),
                                 std::back_inserter (pEvenVector),
                                 std::back_inserter (pOddVector),
                                 [&ctoggle] (int)
            {
                return ctoggle = !ctoggle;
            });
        }

        void getOddElements (std::vector<uint32_t> pInputVector, std::vector<uint32_t>& pOddVector)
        {
            bool ctoggle = true;
            std::copy_if (pInputVector.begin(),
                                 pInputVector.end(),
                                 std::back_inserter (pOddVector),
                                 [&ctoggle] (int)
            {
                return ctoggle = !ctoggle;
            });
        }


      public:
        ///////////////////////////////////////////////////////
        //      CBC Methods                                 //
        /////////////////////////////////////////////////////

        //Encode/Decode Cbc values
        /*!
        * \brief Encode a/several word(s) readable for a Cbc
        * \param pRegItem : RegItem containing infos (name, adress, value...) about the register to write
        * \param pCbcId : Id of the Cbc to work with
        * \param pVecReq : Vector to stack the encoded words
        */
        void EncodeReg ( const CbcRegItem& pRegItem, uint8_t pCbcId, std::vector<uint32_t>& pVecReq, bool pRead, bool pWrite ) override; /*!< Encode a/several word(s) readable for a Cbc*/
        void EncodeReg ( const CbcRegItem& pRegItem, uint8_t pFeId, uint8_t pCbcId, std::vector<uint32_t>& pVecReq, bool pRead, bool pWrite ) override; /*!< Encode a/several word(s) readable for a Cbc*/
        void BCEncodeReg ( const CbcRegItem& pRegItem, uint8_t pNCbc, std::vector<uint32_t>& pVecReq, bool pRead, bool pWrite ) override;
        void DecodeReg ( CbcRegItem& pRegItem, uint8_t& pCbcId, uint32_t pWord, bool& pRead, bool& pFailed ) override;


        bool WriteCbcBlockReg ( std::vector<uint32_t>& pVecReg, bool pReadback) override;
        bool BCWriteCbcBlockReg ( std::vector<uint32_t>& pVecReg, bool pReadback) override;
        void ReadCbcBlockReg (  std::vector<uint32_t>& pVecReg );

        void CbcHardReset();

        void CbcFastReset();

        void CbcI2CRefresh();

        void CbcTestPulse();

        void CbcTrigger();
        ///////////////////////////////////////////////////////
        //      FPGA CONFIG                                 //
        /////////////////////////////////////////////////////

        /*! \brief Upload a firmware (FPGA configuration) from a file in MCS format into a given configuration
         * \param numConfig FPGA configuration number (1 or 2)
         * \param pstrFile path to MCS file
         */
        void FlashProm ( const std::string& strConfig, const char* pstrFile );
        /*! \brief Jump to an FPGA configuration */
        void JumpToFpgaConfig ( const std::string& strConfig);
        /*! \brief Is the FPGA being configured ?
         * \return FPGA configuring process or NULL if configuration occurs */
        const FpgaConfig* getConfiguringFpga()
        {
            return fpgaConfig;
        }

    };
}

#endif
