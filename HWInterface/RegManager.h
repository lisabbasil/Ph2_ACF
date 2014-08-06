/*!

    \file                         RegManager.h
    \brief                        RegManager class, permit connection & r/w registers
    \author                       Nicolas PIERRE
    \version                      0.3
    Date of creation :            06/06/14
    Support :                     mail to : nicolas.pierre@cern.ch

*/
#ifndef __REGMANAGER_H__
#define __REGMANAGER_H__

#include <string>
#include <map>
#include <vector>
#include <utility>
#include <thread>
#include <mutex>
#include <chrono>
#include <uhal/uhal.hpp>

/*!
* \namespace Ph2_HwInterface
* \brief Namespace regrouping all the interfaces to the hardware
*/
namespace Ph2_HwInterface
{
    /*!
    * \class RegManager
    * \brief Permit connection to given boards and r/w given registers
    */
    class RegManager
    {
        protected:
            uhal::HwInterface *fBoard; /*!< Board in use*/
            const char *fUHalConfigFileName; /*!< path of the uHal Config File*/
            std::vector< std::pair<std::string,uint32_t> > fStackReg; /*!< Stack of registers*/
            std::thread fThread; /*!< Thread for timeout stack writing*/
            bool fDeactiveThread; /*!< Bool to terminate the thread in the destructor*/
            std::mutex fBoardMutex; /*!< Mutex to avoid conflict btw threads on shared resources*/

        public:
            /*!
            * \brief Write a register
            * \param pRegNode : Node of the register to write
            * \param pVal : Value to write
            * \return boolean confirming the writing
            */
            virtual bool WriteReg(const std::string& pRegNode, const uint32_t& pVal);
            /*!
            * \brief Write a stack of registers
            * \param pVecReg : vector containing the registers and the associated values to write
            * \return boolean confirming the writing
            */
            virtual bool WriteStackReg(std::vector<std::pair<std::string,uint32_t> >& pVecReg);
            /*!
            * \brief Write a block of values in a register
            * \param pRegNode : Node of the register to write
            * \param pValues : Block of values to write
            * \return boolean confirming the writing
            */
            virtual bool WriteBlockReg(const std::string& pRegNode, const std::vector< uint32_t >& pValues);
            /*!
            * \brief Read a value in a register
            * \param pRegNode : Node of the register to read
            * \return ValWord value of the register
            */
            virtual uhal::ValWord<uint32_t> ReadReg(const std::string& pRegNode);
            /*!
            * \brief Read a block of values in a register
            * \param pRegNode : Node of the register to read
            * \param pBlocksize : Size of the block to read
            * \return ValVector block values of the register
            */
            virtual uhal::ValVector<uint32_t> ReadBlockReg(const std::string& pRegNode, const uint32_t& pBlocksize);
            /*!
            * \brief Time Out for sending the register/value stack in the writting.
            * \brief It has only to be set in a detached thread from the one you're working on
            */
            virtual void StackWriteTimeOut();

        public:
            // Connection w uHal
            /*!
            * \brief Constructor of the RegManager class
            * \param puHalConfigFileName : path of the uHal Config File
            */
            RegManager(const char *puHalConfigFileName, uint32_t pBoardId);
            /*!
            * \brief Destructor of the RegManager class
            */
            virtual ~RegManager();
            /*!
            * \brief Stack the commands, deliver when full or timeout
            * \param pRegNode : Register to write
            * \param pVal : Value to write
            * \param pSend : Send the stack to write or nor (1/0)
            */
            virtual void StackReg(const std::string& pRegNode, const uint32_t& pVal, bool pSend=false);

    };
}

#endif
