/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2022 Northeastern University
 * Copyright (c) 2022 Sapienza, University of Rome
 * Copyright (c) 2022 University of Padova
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Andrea Lacava <thecave003@gmail.com>
 *		   Tommaso Zugno <tommasozugno@gmail.com>
 *		   Michele Polese <michele.polese@gmail.com>
 */
 
#include <ns3/ric-control-message.h>
#include <ns3/asn1c-types.h>
#include <ns3/log.h>
#include <bitset>
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RicControlMessage");


RicControlMessage::RicControlMessage (E2AP_PDU_t* pdu)
{
  NS_LOG_INFO("Start of RicControlMessage::RicControlMessage()");
  DecodeRicControlMessage (pdu);
  NS_LOG_INFO("End of RicControlMessage::RicControlMessage()");
}

RicControlMessage::~RicControlMessage ()
{

}

void  
RicControlMessage::DecodeRicControlMessage(E2AP_PDU_t* pdu)
{
    InitiatingMessage_t* mess = pdu->choice.initiatingMessage;
    auto *request = (RICcontrolRequest_t *) &mess->value.choice.RICcontrolRequest;
    NS_LOG_INFO (xer_fprint(stderr, &asn_DEF_RICcontrolRequest, request));

    size_t count = request->protocolIEs.list.count; 
    if (count <= 0) {
        NS_LOG_ERROR("[E2SM] received empty list");
        return;
    }

    for (size_t i = 0; i < count; i++) 
    {
        RICcontrolRequest_IEs_t *ie = request->protocolIEs.list.array [i];
        switch (ie->value.present) {
            case RICcontrolRequest_IEs__value_PR_RICrequestID: {
                NS_LOG_DEBUG("[E2SM] RICcontrolRequest_IEs__value_PR_RICrequestID");
                m_ricRequestId = ie->value.choice.RICrequestID;
                switch (m_ricRequestId.ricRequestorID) {
                    case 1001: {
                        NS_LOG_DEBUG("TS xApp message");
                        m_requestType = ControlMessageRequestIdType::TS;
                        break;
                    }
                    case 1002: {
                        NS_LOG_DEBUG("QoS xApp message");
                        m_requestType = ControlMessageRequestIdType::QoS;
                        break;
                    }
                    case 1024: {
                        NS_LOG_DEBUG("RC xApp message");
                        m_requestType = ControlMessageRequestIdType::RC;
                        break;
                    }
                    default:
                        NS_LOG_DEBUG("Unhandled ricRequestorID\n");
                        m_requestType = static_cast<ControlMessageRequestIdType>(m_ricRequestId.ricRequestorID);
                        break;
                }
                break;
            }
            case RICcontrolRequest_IEs__value_PR_RANfunctionID: {
                m_ranFunctionId = ie->value.choice.RANfunctionID;

                NS_LOG_DEBUG("[E2SM] RICcontrolRequest_IEs__value_PR_RANfunctionID");
                break;
            }
            case RICcontrolRequest_IEs__value_PR_RICcallProcessID: {
                m_ricCallProcessId = ie->value.choice.RICcallProcessID;
                NS_LOG_DEBUG("[E2SM] RICcontrolRequest_IEs__value_PR_RICcallProcessID");
                break;
            }
            case RICcontrolRequest_IEs__value_PR_RICcontrolHeader: {
                NS_LOG_DEBUG("[E2SM] RICcontrolRequest_IEs__value_PR_RICcontrolHeader");
                // xer_fprint(stderr, &asn_DEF_RICcontrolHeader, &ie->value.choice.RICcontrolHeader);

                auto *e2smControlHeader = (E2SM_RC_ControlHeader_t *) calloc(1,
                                                                             sizeof(E2SM_RC_ControlHeader_t));
                ASN_STRUCT_RESET(asn_DEF_E2SM_RC_ControlHeader, e2smControlHeader);
                asn_dec_rval_t rval = asn_decode(nullptr, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2SM_RC_ControlHeader,
                                     (void **) &e2smControlHeader,
                                     ie->value.choice.RICcontrolHeader.buf,
                                     ie->value.choice.RICcontrolHeader.size);
                // Check if decoding was successful
                if (rval.code != RC_OK) {
                    NS_LOG_ERROR("[E2SM] Error decoding RICcontrolHeader");
                    break;
                }


                NS_LOG_INFO (xer_fprint (stderr, &asn_DEF_E2SM_RC_ControlHeader, e2smControlHeader));
                if (e2smControlHeader->ric_controlHeader_formats.present == E2SM_RC_ControlHeader__ric_controlHeader_formats_PR_controlHeader_Format1) {
                    m_e2SmRcControlHeaderFormat1 = e2smControlHeader->ric_controlHeader_formats.choice.controlHeader_Format1;
                    //m_e2SmRcControlHeaderFormat1->ric_ControlAction_ID;
                    //m_e2SmRcControlHeaderFormat1->ric_ControlStyle_Type;
                    //m_e2SmRcControlHeaderFormat1->ueId;
                } else {
                    NS_LOG_DEBUG("[E2SM] Error in checking format of E2SM Control Header");
                }
                break;
                 // Free the allocated memory for e2smControlHeader if needed
                 ASN_STRUCT_FREE(asn_DEF_E2SM_RC_ControlHeader, e2smControlHeader);

            }
            case RICcontrolRequest_IEs__value_PR_RICcontrolMessage: {
                /*
                NS_LOG_DEBUG("[E2SM] RICcontrolRequest_IEs__value_PR_RICcontrolMessage");
                // xer_fprint(stderr, &asn_DEF_RICcontrolMessage, &ie->value.choice.RICcontrolMessage);

                auto *e2SmControlMessage = (E2SM_RC_ControlMessage_t *) calloc(1,
                                                                               sizeof(E2SM_RC_ControlMessage_t));
                ASN_STRUCT_RESET(asn_DEF_E2SM_RC_ControlMessage, e2SmControlMessage);
                // Decode message then assign to e2SmControlMessage with sutable format.
                asn_decode (nullptr, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2SM_RC_ControlMessage,
                            (void **) &e2SmControlMessage, ie->value.choice.RICcontrolMessage.buf,
                            ie->value.choice.RICcontrolMessage.size);

                NS_LOG_DEBUG("********** START Print Message and Size");
                NS_LOG_DEBUG(ie->value.present);

                NS_LOG_DEBUG(ie->value.choice.RICcontrolMessage.size);
                int buf_lennnn = static_cast<int>(ie->value.choice.RICcontrolMessage.size);
                for(int i =0 ;i <  buf_lennnn; ++i) {

                NS_LOG_DEBUG(static_cast<int>(ie->value.choice.RICcontrolMessage.buf[i]));

                }
                NS_LOG_DEBUG("********** END Print Message and Size");

                NS_LOG_INFO (xer_fprint(stderr, &asn_DEF_E2SM_RC_ControlMessage, e2SmControlMessage));

                NS_LOG_DEBUG("****  e2SmControlMessage->present **** ");
                NS_LOG_DEBUG(e2SmControlMessage->ric_controlMessage_formats.present);

                const bool DISABLE_FOR_OCTANT_STRING = false;
                if (e2SmControlMessage->ric_controlMessage_formats.present == E2SM_RC_ControlMessage__ric_controlMessage_formats_PR_controlMessage_Format1)
                  {
                    m_e2SmRcControlMessageFormat1 = e2SmControlMessage->ric_controlMessage_formats.choice.controlMessage_Format1;



                    if(DISABLE_FOR_OCTANT_STRING) {
                        NS_LOG_DEBUG ("[E2SM] E2SM_RC_ControlMessage_PR_controlMessage_Format1");
                        E2SM_RC_ControlMessage_Format1_t *e2SmRcControlMessageFormat1 = (E2SM_RC_ControlMessage_Format1_t*) calloc(0, sizeof(E2SM_RC_ControlMessage_Format1_t));

                        e2SmRcControlMessageFormat1 = e2SmControlMessage->ric_controlMessage_formats.choice.controlMessage_Format1;
                        NS_LOG_INFO (xer_fprint(stderr, &asn_DEF_E2SM_RC_ControlMessage_Format1, e2SmRcControlMessageFormat1));
                        NS_LOG_DEBUG("*** DONE e2SmControlMessage->choice.controlMessage_Format1 **");
                        
                        assert(e2SmRcControlMessageFormat1 != nullptr && " e2SmRcControlMessageFormat1 is Null");
                        
                        NS_LOG_DEBUG("*** DONE ASSERT ExtractRANParametersFromControlMessage **");
                        m_valuesExtracted =
                            RicControlMessage::ExtractRANParametersFromControlMessage (e2SmRcControlMessageFormat1);
                        
                        NS_LOG_DEBUG("*** DONE ExtractRANParametersFromControlMessage **");
                        if (m_requestType == ControlMessageRequestIdType::TS)
                        {
                            // Get and parse the secondaty cell id according to 3GPP TS 38.473, Section 9.2.2.1
                            for (RANParameterItem item : m_valuesExtracted)
                            {
                                if (item.m_valueType == RANParameterItem::ValueType::OctectString)
                                {
                                    // First 3 digits are the PLMNID (always 111), last digit is CellId
                                    std::string cgi = item.m_valueStr->DecodeContent ();
                                    NS_LOG_INFO ("Decoded CGI value is: " << cgi);
                                    m_secondaryCellId = cgi.back();
                                }
                            }         
                        }
                    }
                    */ 
                    // add 1208
                    NS_LOG_DEBUG("[E2SM] RICcontrolRequest_IEs__value_PR_RICcontrolMessage");

                    auto &cm = ie->value.choice.RICcontrolMessage;

                    // 0. NULL / size 체크 (이제는 통과하고 있는 상태인 듯)
                    if (cm.buf == nullptr || cm.size == 0) {
                        NS_LOG_ERROR("[E2SM] ControlMessage is empty");
                        break;
                    }

                    NS_LOG_DEBUG("[E2SM] Raw ControlMessage buf size = " << cm.size);

                    auto *e2SmControlMessage =
                        (E2SM_RC_ControlMessage_t *) calloc(1, sizeof(E2SM_RC_ControlMessage_t));
                    ASN_STRUCT_RESET(asn_DEF_E2SM_RC_ControlMessage, e2SmControlMessage);
                    /*
                    asn_dec_rval_t rval = asn_decode(
                        nullptr,
                        ATS_ALIGNED_BASIC_PER,
                        &asn_DEF_E2SM_RC_ControlMessage,
                        (void **) &e2SmControlMessage,
                        ie->value.choice.RICcontrolMessage.buf,
                        ie->value.choice.RICcontrolMessage.size);

                    */                  
                    asn_dec_rval_t rval = asn_decode(
                        nullptr,
                        ATS_ALIGNED_BASIC_PER,
                        &asn_DEF_E2SM_RC_ControlMessage,
                        (void **) &e2SmControlMessage,
                        cm.buf,
                        cm.size
                    );

                    NS_LOG_ERROR("[E2SM] Decode ControlMessage: rval.code=" << rval.code
                        << " consumed=" << rval.consumed
                        << " size=" << cm.size);
                    if (rval.code != RC_OK) {
                        ASN_STRUCT_FREE(asn_DEF_E2SM_RC_ControlMessage, e2SmControlMessage);
                        break;   // ❗ 실패했으면 절대 밑으로 내려가면 안 됨
                    }

                    // 🔥 디코딩된 ControlMessage 전체 구조를 XER로 찍기
                    NS_LOG_DEBUG ("[E2SM] Decoded E2SM_RC_ControlMessage (XER):");
                    NS_LOG_DEBUG(xer_fprint(stderr, &asn_DEF_E2SM_RC_ControlMessage, e2SmControlMessage));

                    if (e2SmControlMessage->ric_controlMessage_formats.present ==
                        E2SM_RC_ControlMessage__ric_controlMessage_formats_PR_controlMessage_Format1)
                    {
                        NS_LOG_DEBUG("[E2SM] ControlMessage Format1 detected");

                        m_e2SmRcControlMessageFormat1 =
                            e2SmControlMessage->ric_controlMessage_formats.choice.controlMessage_Format1;

                        int cnt = 0;
                        if (m_e2SmRcControlMessageFormat1->ranP_List.list.array != nullptr) {
                            cnt = m_e2SmRcControlMessageFormat1->ranP_List.list.count;
                        }
                        NS_LOG_DEBUG("[E2SM] ControlMessage Format1 ranP_List.count = " << cnt);

                        // 각 항목의 ranParameter-ID도 한 번 찍어보자 (있다면)
                        for (int i = 0; i < cnt; ++i) {
                            auto *item = m_e2SmRcControlMessageFormat1->ranP_List.list.array[i];
                            if (item) {
                                NS_LOG_DEBUG("[E2SM] ranP_List[" << i
                                            << "].ranParameter-ID = "
                                            << item->ranParameter_ID);
                            } else {
                                NS_LOG_DEBUG("[E2SM] ranP_List[" << i << "] is NULL");
                            }
                        }
                    

                  }
                else
                  {
                    NS_LOG_DEBUG("[E2SM] Error in checking format of E2SM Control Message");
                  }
                break;
            }
            case RICcontrolRequest_IEs__value_PR_RICcontrolAckRequest: {
                NS_LOG_DEBUG("[E2SM] RICcontrolRequest_IEs__value_PR_RICcontrolAckRequest");

                switch (ie->value.choice.RICcontrolAckRequest) {
                    case RICcontrolAckRequest_noAck: {
                        NS_LOG_DEBUG("[E2SM] RIC Control ack value: NO ACK");
                        break;
                    }
                    case RICcontrolAckRequest_ack: {
                        NS_LOG_DEBUG("[E2SM] RIC Control ack value: ACK");
                        break;
                    }
                    
                    default: {
                        NS_LOG_DEBUG("[E2SM] RIC Control ack value unknown");
                        break;
                    }
                }
                break;
            }
            case RICcontrolRequest_IEs__value_PR_NOTHING: {
                NS_LOG_DEBUG("[E2SM] RICcontrolRequest_IEs__value_PR_NOTHING");
                NS_LOG_DEBUG("[E2SM] Nothing");
                break;
            }
            default: {
                NS_LOG_DEBUG("[E2SM] RIC Control value unknown");
                break;
            }
        }
    }

    NS_LOG_INFO ("End of DecodeRicControlMessage");
}

std::string
RicControlMessage::GetSecondaryCellIdHO ()
{
  return m_secondaryCellId;
}
/*
std::vector<RANParameterItem> RicControlMessage::ExtractRANParametersFromControlMessage (
      E2SM_RC_ControlMessage_Format1_t *e2SmRcControlMessageFormat1) 
{
  NS_LOG_DEBUG("***** ExtractRANParametersFromControlMessage 0 ****");
  std::vector<RANParameterItem> ranParameterList;
  NS_LOG_DEBUG("***** ExtractRANParametersFromControlMessage 1 *****");

  assert(e2SmRcControlMessageFormat1->ranP_List != nullptr && " ranParametersList is null");
  // Not implemeted in the FlexRIC side 
  int count = e2SmRcControlMessageFormat1->ranP_List.list.count;

  NS_LOG_DEBUG("***** ExtractRANParametersFromControlMessage 2 *****");
  for (int i = 0; i < count; i++)
    {
      RANParameter_Item_t *ranParameterItem =
          e2SmRcControlMessageFormat1->ranP_List.list.array[i];
      for (RANParameterItem extractedParameter :
           RANParameterItem::ExtractRANParametersFromRANParameter (ranParameterItem))
        {
          ranParameterList.push_back (extractedParameter);
        }
    }

  return ranParameterList;
}
*/
// add with gpt
uint16_t
RicControlMessage::GetTargetCell () const
{
  // -------------------------------
  // 0. Format1 존재 확인
  // -------------------------------
  if (!m_e2SmRcControlMessageFormat1)
    {
      NS_LOG_ERROR ("GetTargetCell(): Format1 NULL");
      return 0;
    }

  auto &topList = m_e2SmRcControlMessageFormat1->ranP_List.list;

  if (topList.count == 0 || !topList.array)
    {
      NS_LOG_ERROR ("GetTargetCell(): ranP_List empty");
      return 0;
    }


  // -------------------------------
  // 1. Top-level RANParameter 구조
  // -------------------------------
  auto *topItem = topList.array[0];
  if (!topItem)
    {
      NS_LOG_ERROR ("GetTargetCell(): topItem NULL");
      return 0;
    }

  if (topItem->ranParameter_valueType.present !=
      RANParameter_ValueType_PR_ranP_Choice_Structure)
    {
      NS_LOG_ERROR ("GetTargetCell(): topItem not structure");
      return 0;
    }

  auto *topStructChoice =
      topItem->ranParameter_valueType.choice.ranP_Choice_Structure;

  if (!topStructChoice || !topStructChoice->ranParameter_Structure)
    {
      NS_LOG_ERROR ("GetTargetCell(): top structure missing");
      return 0;
    }

  auto *topStruct = topStructChoice->ranParameter_Structure;

  if (!topStruct->sequence_of_ranParameters ||
      topStruct->sequence_of_ranParameters->list.count == 0)
    {
      NS_LOG_ERROR ("GetTargetCell(): topStruct empty");
      return 0;
    }


  // -------------------------------
  // 2. TargetCell 구조
  // -------------------------------
  auto *targetCellItem =
      topStruct->sequence_of_ranParameters->list.array[0];

  if (!targetCellItem ||
      targetCellItem->ranParameter_valueType->present !=
      RANParameter_ValueType_PR_ranP_Choice_Structure)
    {
      NS_LOG_ERROR ("GetTargetCell(): targetCellItem invalid");
      return 0;
    }

  auto *targetCellChoice =
      targetCellItem->ranParameter_valueType->choice.ranP_Choice_Structure;

  if (!targetCellChoice || !targetCellChoice->ranParameter_Structure)
    {
      NS_LOG_ERROR ("GetTargetCell(): targetCellStruct missing");
      return 0;
    }

  auto *targetCellStruct = targetCellChoice->ranParameter_Structure;

  if (!targetCellStruct->sequence_of_ranParameters ||
      targetCellStruct->sequence_of_ranParameters->list.count == 0)
    {
      NS_LOG_ERROR ("GetTargetCell(): targetCellStruct empty");
      return 0;
    }


  // -------------------------------
  // 3. NR Cell 구조
  // -------------------------------
  auto *nrCellItem =
      targetCellStruct->sequence_of_ranParameters->list.array[0];

  if (!nrCellItem ||
      nrCellItem->ranParameter_valueType->present !=
      RANParameter_ValueType_PR_ranP_Choice_Structure)
    {
      NS_LOG_ERROR ("GetTargetCell(): nrCellItem invalid");
      return 0;
    }

  auto *nrCellChoice =
      nrCellItem->ranParameter_valueType->choice.ranP_Choice_Structure;

  if (!nrCellChoice || !nrCellChoice->ranParameter_Structure)
    {
      NS_LOG_ERROR ("GetTargetCell(): nrCellStruct missing");
      return 0;
    }

  auto *nrCellStruct = nrCellChoice->ranParameter_Structure;

  if (!nrCellStruct->sequence_of_ranParameters ||
      nrCellStruct->sequence_of_ranParameters->list.count == 0)
    {
      NS_LOG_ERROR ("GetTargetCell(): nrCellStruct empty");
      return 0;
    }


  // -------------------------------
  // 4. NR CGI (ElementFalse)
  // -------------------------------
  auto *nrCgiItem =
      nrCellStruct->sequence_of_ranParameters->list.array[0];

  if (!nrCgiItem ||
      nrCgiItem->ranParameter_valueType->present !=
      RANParameter_ValueType_PR_ranP_Choice_ElementFalse)
    {
      NS_LOG_ERROR ("GetTargetCell(): nrCgiItem invalid");
      return 0;
    }

  auto *elemFalse =
      nrCgiItem->ranParameter_valueType->choice.ranP_Choice_ElementFalse;

  if (!elemFalse || !elemFalse->ranParameter_value)
    {
      NS_LOG_ERROR ("GetTargetCell(): elemFalse NULL");
      return 0;
    }

  auto *val = elemFalse->ranParameter_value;

  NS_LOG_DEBUG("GetTargetCell(): valueType present=" << val->present);


  // -------------------------------
  // 5. valueBitS / valueOctS / valueInt 모두 지원
  // -------------------------------
  uint16_t cellId = 0;

  switch (val->present)
    {
    case RANParameter_Value_PR_valueBitS:
      {
        BIT_STRING_t bits = val->choice.valueBitS;

        if (!bits.buf || bits.size == 0)
          {
            NS_LOG_ERROR("GetTargetCell(): valueBitS empty");
            return 0;
          }

        uint8_t lastByte = bits.buf[bits.size - 1];
        cellId = lastByte & 0x0F;
        break;
      }

    case RANParameter_Value_PR_valueOctS:
      {
        OCTET_STRING_t oct = val->choice.valueOctS;

        if (!oct.buf || oct.size == 0)
          {
            NS_LOG_ERROR("GetTargetCell(): valueOctS empty");
            return 0;
          }

        uint8_t lastByte = oct.buf[oct.size - 1];
        cellId = lastByte & 0x0F;
        break;
      }

    case RANParameter_Value_PR_valueInt:
      {
        long v = val->choice.valueInt;
        cellId = v & 0x0F;
        break;
      }

    default:
      NS_LOG_ERROR("GetTargetCell(): Unsupported type=" << val->present);
      return 0;
    }


  // -------------------------------
  // 6. 결과 출력
  // -------------------------------
  NS_LOG_DEBUG("GetTargetCell(): decoded cellId=" << cellId);
  return cellId;
}



uint64_t
RicControlMessage::GetUeId() const
{
    auto *hdr = m_e2SmRcControlHeaderFormat1;
    if (!hdr) {
        NS_LOG_ERROR("ControlHeaderFormat1 is NULL");
        return 0;
    }

    if (hdr->ueID.present != UEID_PR_gNB_UEID) {
        NS_LOG_ERROR("UEID is not gNB_UEID type!");
        return 0;
    }

    UEID_GNB_t *ue = hdr->ueID.choice.gNB_UEID;
    if (!ue) {
        NS_LOG_ERROR("UEID_GNB is NULL");
        return 0;
    }

    // gNB-CU-UE-F1AP-ID-List → 첫 item 사용
    auto *f1List = ue->gNB_CU_UE_F1AP_ID_List;

    if (!f1List || f1List->list.count == 0) {
        NS_LOG_ERROR("gNB-CU-UE-F1AP-ID list empty!");
        return 0;
    }

    auto *item = f1List->list.array[0];
    if (!item) {
        NS_LOG_ERROR("gNB-CU-UE-F1AP-ID item is NULL!");
        return 0;
    }

    uint64_t ueId = item->gNB_CU_UE_F1AP_ID;

    NS_LOG_DEBUG("Parsed UE-ID = " << ueId);
    return ueId;
}



} // namespace ns3
