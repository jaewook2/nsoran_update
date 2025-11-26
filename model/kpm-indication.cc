/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2022 Northeastern University
 * Copyright (c) 2022 Sapienza, University of Rome
 * Copyright (c) 2022 University of Padova
 * Copyright (c) 2024 Orange Innovation Egypt
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
 *       Mina Yonan <mina.awadallah@orange.com>
 *       Mostafa Ashraf <mostafa.ashraf.ext@orange.com>
 *       Aya Kamal <aya.kamal.ext@orange.com>
 *       Abdelrhman Soliman <abdelrhman.soliman.ext@orange.com>
 */

#include <ns3/kpm-indication.h>
// #include "kpm-indication.h"

#include <ns3/asn1c-types.h>
#include <ns3/log.h>

extern "C" {
  #include <arpa/inet.h>

  #include "conversions.h"
  #include <assert.h>
  #include <cassert>

  // #include "timing_ms.h"

  BIT_STRING_t cp_amf_region_id_to_bit_string (uint8_t src);

  uint8_t cp_amf_region_id_to_u8 (BIT_STRING_t src);

  /* AMF Set ID */
  BIT_STRING_t cp_amf_set_id_to_bit_string (uint16_t val);

  uint16_t cp_amf_set_id_to_u16 (BIT_STRING_t src);

  /* AMF Pointer */
  BIT_STRING_t cp_amf_ptr_to_bit_string (uint8_t src);

  uint8_t cp_amf_ptr_to_u8 (BIT_STRING_t src);

  OCTET_STRING_t cp_plmn_identity_to_octant_string (uint8_t src);
}

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("KpmIndication");

KpmIndicationHeader::KpmIndicationHeader (GlobalE2nodeType nodeType,
                                          KpmRicIndicationHeaderValues values)
{
  m_nodeType = nodeType;
  E2SM_KPM_IndicationHeader_t *descriptor = new E2SM_KPM_IndicationHeader_t;
  FillAndEncodeKpmRicIndicationHeader (descriptor, values);
  delete descriptor;
}

KpmIndicationHeader::~KpmIndicationHeader ()
{
  NS_LOG_FUNCTION (this);
  free (m_buffer);
  m_size = 0;
}

uint64_t
KpmIndicationHeader::time_now_us_clck ()
{
  struct timespec tms;

  /* The C11 way */
  /* if (! timespec_get(&tms, TIME_UTC))  */

  /* POSIX.1-2008 way */
  if (clock_gettime (CLOCK_REALTIME, &tms))
    {
      return -1;
    }
  /* seconds, multiplied with 1 million */
  uint64_t micros = tms.tv_sec * 1000000;
  /* Add full microseconds */
  micros += tms.tv_nsec / 1000;
  /* round up if necessary */
  if (tms.tv_nsec % 1000 >= 500)
    {
      ++micros;
    }
  NS_LOG_INFO ("**micros Timing is " << micros);
  return micros;
}

// KPM ind_msg latency
OCTET_STRING_t
KpmIndicationHeader::get_time_now_us ()
{

  std::vector<uint8_t> byteArray (sizeof (uint64_t));

  auto val = time_now_us_clck ();
  memcpy (byteArray.data (), &val, sizeof (uint64_t));
  OCTET_STRING_t dst = {0};

  dst.buf = (uint8_t *) calloc (byteArray.size (), sizeof (uint8_t));
  dst.size = byteArray.size ();

  memcpy (dst.buf, byteArray.data (), dst.size);

  return dst;
}

OCTET_STRING_t
KpmIndicationHeader::int_64_to_octet_string (uint64_t x)
{
  OCTET_STRING_t asn = {0};

  asn.buf = (uint8_t *) calloc (sizeof (x) + 1, sizeof (char));
  memcpy (asn.buf, &x, sizeof (x));
  asn.size = sizeof (x);

  return asn;
}

uint64_t
KpmIndicationHeader::octet_string_to_int_64 (OCTET_STRING_t asn)
{
  uint64_t x = {0};

  memcpy (&x, asn.buf, asn.size);

  return x;
}

void
KpmIndicationHeader::Encode (E2SM_KPM_IndicationHeader_t *descriptor)
{
  // 기존 버퍼가 있으면 해제 (재인코딩 대비)
  if (m_buffer) {
    free(m_buffer);
    m_buffer = nullptr;
    m_size   = 0;
  }
  asn_codec_ctx_t *opt_cod = nullptr; // disable stack bounds checking

  asn_encode_to_new_buffer_result_s encodedHeader = asn_encode_to_new_buffer (
      opt_cod, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2SM_KPM_IndicationHeader, descriptor);

  if (encodedHeader.result.encoded < 0 || encodedHeader.buffer == nullptr) {
    const char *ft = encodedHeader.result.failed_type
                       ? encodedHeader.result.failed_type->name
                       : "unknown";
    NS_FATAL_ERROR("*Error during encoding Indication Header, errno: "
                   << strerror(errno) << ", failed_type " << ft
                   << ", structure_ptr " << encodedHeader.result.structure_ptr);
  }

  m_buffer = encodedHeader.buffer;
  m_size = encodedHeader.result.encoded;
}

// ntohll 함수는 “네트워크(빅엔디안) ↔ 호스트” 변환을 **반쪽(32비트)씩 ntohl**로 처리
uint64_t
ntohll (uint64_t val)
{
  uint32_t high = ntohl ((uint32_t) (val >> 32)); // Convert high 32 bits
  uint32_t low = ntohl ((uint32_t) (val & 0xFFFFFFFF)); // Convert low 32 bits
  return ((uint64_t) high << 32) | low; // Combine the high and low 32 bits back together
}

void
KpmIndicationHeader::FillAndEncodeKpmRicIndicationHeader (E2SM_KPM_IndicationHeader_t *descriptor,
                                                          KpmRicIndicationHeaderValues values)
{
    NS_LOG_INFO("FillAndEncodeKpmRicIndicationHeader");

  E2SM_KPM_IndicationHeader_Format1_t* ind_header =
      (E2SM_KPM_IndicationHeader_Format1_t*)calloc(1, sizeof(E2SM_KPM_IndicationHeader_Format1_t));

  // ========= colletStartTime: values.m_timestamp 를 반영 =========
  // 가정: values.m_timestamp 는 uint64_t, 단위는 us(마이크로초)
  uint64_t host_time = values.m_timestamp;

  // 만약 0으로 들어오면 fallback 으로 현재 시간 사용
  if (host_time == 0)
    {
      TimeStamp_t colstartTime = get_time_now_us();
      memcpy(&host_time, colstartTime.buf, sizeof(uint64_t));

      if (colstartTime.buf)
        {
          free(colstartTime.buf);
          colstartTime.buf  = nullptr;
          colstartTime.size = 0;
        }
    }

  // 호스트 → 네트워크 바이트 오더
#if defined(_WIN32)
  uint64_t net_time = _byteswap_uint64(host_time);
#else
  uint64_t net_time = htobe64(host_time);
#endif

  ind_header->colletStartTime.buf  = (uint8_t*)calloc(8, 1);
  ind_header->colletStartTime.size = 8;
  memcpy(ind_header->colletStartTime.buf, &net_time, 8);

  NS_LOG_INFO("colletStartTime (host) = " << host_time);

  /*
  NS_LOG_INFO ("FillAndEncodeKpmRicIndicationHeader");


  E2SM_KPM_IndicationHeader_Format1_t* ind_header = 
    (E2SM_KPM_IndicationHeader_Format1_t *) calloc (1, sizeof (E2SM_KPM_IndicationHeader_Format1_t));


  TimeStamp_t colstartTime = get_time_now_us();

  // OCTET_STRING -> uint64
  uint64_t host_time = 0;
  memcpy (&host_time, colstartTime.buf, sizeof (uint64_t));
  // 호스트 → 네트워크 바이트오더
  uint64_t net_time  = ntohll (host_time);

  // in-place 채우기
  ind_header->colletStartTime.buf  = (uint8_t*)calloc(8, 1);
  ind_header->colletStartTime.size = 8;
  memcpy(ind_header->colletStartTime.buf, &net_time, 8);

  NS_LOG_INFO (" colletStartTime " << octet_string_to_int_64 (ind_header->colletStartTime));
  if (colstartTime.buf) {
    free(colstartTime.buf);
    colstartTime.buf  = nullptr;
    colstartTime.size = 0;
  }
*/

// 헤더에 연결

  /*
  // === GlobalE2node-ID 를 in-place로 채움 (별도 버퍼 만들지 않음) ===
  GlobalE2node_ID_t& id = ind_header->id_GlobalE2node_ID;  // by reference (in-place)

  Ptr<OctetString> plmnid = Create<OctetString> (values.m_plmId, 3);
  Ptr<BitString> cellId_bstring;
  switch (m_nodeType)
    {
      case gNB: {
        NS_LOG_INFO ("gNB Header");
        static int sizeGnb = 4; // 3GPP Specs
        cellId_bstring = Create<BitString> (values.m_gnbId, sizeGnb);
        id.present = GlobalE2node_ID_PR_gNB;

        globalE2nodeIdBuf->present = GlobalE2node_ID_PR_gNB;

        GlobalE2node_gNB_ID_t* gnb =
            (GlobalE2node_gNB_ID_t*)calloc(1, sizeof(*gnb));
        gnb->global_gNB_ID.plmn_id                 = plmnid->GetValue();
        gnb->global_gNB_ID.gnb_id.present          = GNB_ID_Choice_PR_gnb_ID;
        gnb->global_gNB_ID.gnb_id.choice.gnb_ID    = cellId_bstring->GetValue();

        id.choice.gNB = gnb;
        break;
      }

      case eNB: {
        // Use this.
        NS_LOG_INFO ("eNB Header");
        static int sizeEnb = 3;  // 36.413
        static int unused   = 4;
        cellId_bstring = Create<BitString>(values.m_gnbId, sizeEnb, unused);

        id.present = GlobalE2node_ID_PR_eNB;

        GlobalE2node_eNB_ID_t* enb =
            (GlobalE2node_eNB_ID_t*)calloc(1, sizeof(*enb));
        enb->global_eNB_ID.pLMN_Identity          = plmnid->GetValue();
        enb->global_eNB_ID.eNB_ID.present         = E2AP_IEs_ENB_ID_PR_macro_eNB_ID;
        enb->global_eNB_ID.eNB_ID.choice.macro_eNB_ID = cellId_bstring->GetValue();

        id.choice.eNB = enb;
        break;
      }

      case ng_eNB: {
        NS_LOG_INFO ("ng_eNB Header");

        static int sizeEnb = 3; // 3GPP TS 36.413 version 14.8.0 Release 14, Section 9.2.1.37 Global eNB ID
        static int unused   = 4;
        cellId_bstring = Create<BitString> (values.m_gnbId, sizeEnb, unused);

        globalE2nodeIdBuf->present = GlobalE2node_ID_PR_ng_eNB;

        
        id.present = GlobalE2node_ID_PR_ng_eNB;

        GlobalE2node_ng_eNB_ID_t* nge =
            (GlobalE2node_ng_eNB_ID_t*)calloc(1, sizeof(*nge));
        nge->global_ng_eNB_ID.plmn_id            = plmnid->GetValue();
        nge->global_ng_eNB_ID.enb_id.present     = ENB_ID_Choice_PR_enb_ID_macro;
        nge->global_ng_eNB_ID.enb_id.choice.enb_ID_macro = cellId_bstring->GetValue();

        id.choice.ng_eNB = nge;
        break;
      }
      // To be checked 1015
      case en_gNB: {
        NS_LOG_INFO("en_gNB Header");

        static int sizeGnb = 4;                   // 3GPP spec (28-bit gNB ID 등)
        cellId_bstring = Create<BitString>(values.m_gnbId, sizeGnb);

        // in-place로 채움: ind_header->id_GlobalE2node_ID
        id.present = GlobalE2node_ID_PR_en_gNB;

        GlobalE2node_en_gNB_ID_t* engnb =
            (GlobalE2node_en_gNB_ID_t*)calloc(1, sizeof(*engnb));

        // 표준 구조: global_en_gNB_ID.{pLMN_Identity, gNB_ID}
        engnb->global_en_gNB_ID.pLMN_Identity         = plmnid->GetValue();

        // enum 이름은 asn1c 버전에 따라 약간 다릅니다.
        // 예) EN_GNB_ID_Choice_PR_gNB_ID  또는  ENGNB_ID_PR_gNB_ID
        engnb->global_en_gNB_ID.gNB_ID.present        = ENGNB_ID_PR_gNB_ID;
        engnb->global_en_gNB_ID.gNB_ID.choice.gNB_ID  = cellId_bstring->GetValue();

        id.choice.en_gNB = engnb;
        break;
      }

    default:
      NS_FATAL_ERROR ("Unrecognized node type for KpmRicIndicationHeader, value passed: " << m_nodeType);
      break;
    }
  */
  NS_LOG_INFO (xer_fprint (stderr, &asn_DEF_E2SM_KPM_IndicationHeader_Format1, ind_header));

  descriptor->indicationHeader_formats.present =
      E2SM_KPM_IndicationHeader__indicationHeader_formats_PR_indicationHeader_Format1;
  descriptor->indicationHeader_formats.choice.indicationHeader_Format1 = ind_header;

  Encode (descriptor);
  ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationHeader_Format1, ind_header);

}

KpmIndicationMessage::KpmIndicationMessage (KpmIndicationMessageValues values, const E2SM_KPM_IndicationMessage_FormatType &format_type)
{
  E2SM_KPM_IndicationMessage_t* descriptor = (E2SM_KPM_IndicationMessage_t*)calloc(1, sizeof(*descriptor));
  CheckConstraints(values);
  FillAndEncodeKpmIndicationMessage(descriptor, values, format_type); //E2SM_KPM_INDICATION_MESSAGE_FORMART1
  ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationMessage, descriptor);
  /*
  E2SM_KPM_IndicationMessage_t* descriptor3 = (E2SM_KPM_IndicationMessage_t*)calloc(1, sizeof(*descriptor3));
  CheckConstraints(values);
  NS_LOG_DEBUG ("Make UE KPI Massage");
  FillAndEncodeKpmIndicationMessage(descriptor3, values, E2SM_KPM_INDICATION_MESSAGE_FORMART3); //E2SM_KPM_INDICATION_MESSAGE_FORMART1
  ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationMessage, descriptor3);
  */
}

KpmIndicationMessage::~KpmIndicationMessage ()
{
  free (m_buffer);
  m_size = 0;
}

void
KpmIndicationMessage::CheckConstraints (KpmIndicationMessageValues values)
{
  //NS_LOG_DEBUG ("m_cellObjectId=" << values.m_cellObjectId);
}

typedef struct
{
  uint8_t *buf;
  size_t len;
} darsh_byte_array_t;

void
KpmIndicationMessage::Encode (E2SM_KPM_IndicationMessage_t *descriptor)
{

  const bool USE_PRIVATE_BUFFER = false;
  if (USE_PRIVATE_BUFFER)
    {

      darsh_byte_array_t ba_darsh = {.buf = (uint8_t *) malloc (2048), .len = 2048};
      asn_enc_rval_t encodedMsg =
          asn_encode_to_buffer (NULL, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2SM_KPM_IndicationMessage,
                                descriptor, ba_darsh.buf, ba_darsh.len);

      if (encodedMsg.encoded < 0)
        {
          assert (encodedMsg.structure_ptr != nullptr);

          NS_FATAL_ERROR ("Error during the encoding of the RIC Indication Message, errno: "
                          << strerror (errno) << ", failed_type " << encodedMsg.failed_type->name
                          << ", structure_ptr " << encodedMsg.structure_ptr);

          NS_LOG_INFO ("Error during encoding ");
        }

      NS_LOG_LOGIC ("encodedMsg.encoded=" << encodedMsg.encoded);
      NS_LOG_LOGIC ("ba_darsh.len=" << ba_darsh.len);

      assert (encodedMsg.encoded > -1);
      assert ((size_t) encodedMsg.encoded <= ba_darsh.len);

      m_buffer = ba_darsh.buf;
      m_size = encodedMsg.encoded;
    }
  else
    {

      // asn_codec_ctx_t *opt_cod = 0; // disable stack bounds checking
      asn_encode_to_new_buffer_result_s encodedMsg = asn_encode_to_new_buffer (
          NULL, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2SM_KPM_IndicationMessage, descriptor);

      if (encodedMsg.result.encoded < 0)
        {
          assert (encodedMsg.result.structure_ptr != nullptr);

          NS_FATAL_ERROR ("Error during the encoding of the RIC Indication Message, errno: "
                          << strerror (errno) << ", failed_type "
                          << encodedMsg.result.failed_type->name << ", structure_ptr "
                          << encodedMsg.result.structure_ptr);
        }

      m_buffer = encodedMsg.buffer;
      m_size = encodedMsg.result.encoded;
    }
}

// int KpmIndicationMessage::count = 0;
// std::mutex KpmIndicationMessage::mtx;

std::pair<MeasurementInfoItem_t *, MeasurementDataItem_t *>
KpmIndicationMessage::getMesInfoItem (const Ptr<MeasurementItem> &mesItem)
{

  // // 1. Adding a measurement values(item) to the packet.
  //MeasurementDataItem_t *measureDataItem = nullptr;

  auto item = mesItem->GetValue ();

  std::string measName;
  if (item.pmType.present == MeasurementType_PR_measName) {
      measName = std::string(
          (char *)item.pmType.choice.measName.buf,
          item.pmType.choice.measName.size);
  } else {
      NS_LOG_WARN("MeasurementType is not measName, skip");
      return {nullptr, nullptr};
  }
  MeasurementType_t *measType =
      (MeasurementType_t *)calloc(1, sizeof(MeasurementType_t));

  measType->present = MeasurementType_PR_measName;
  OCTET_STRING_fromBuf(&measType->choice.measName,
                        measName.c_str(),
                        measName.size());

  MeasurementDataItem_t *measDataItem = nullptr;

    switch (item.pmVal.present)
    {
    case MeasurementValue_PR_valueInt:
        measDataItem = getMesDataItem((double)item.pmVal.choice.valueInt);
        break;

    case MeasurementValue_PR_valueReal:
        measDataItem = getMesDataItem(item.pmVal.choice.valueReal);
        break;

    default:
        NS_LOG_WARN("Unsupported MeasurementValue type");
        return {nullptr, nullptr};
    }

    // ---------------------------------------------
    // 3) MeasurementLabel 생성 (KPM 표준)
    //    noLabel = true 만 설정
    // ---------------------------------------------
    MeasurementLabel_t *measLabel =
        (MeasurementLabel_t *)calloc(1, sizeof(MeasurementLabel_t));

    measLabel->noLabel = (long *)calloc(1, sizeof(long));
    *measLabel->noLabel = 0;

    LabelInfoItem_t *labelItem =
        (LabelInfoItem_t *)calloc(1, sizeof(LabelInfoItem_t));

    labelItem->measLabel = *measLabel;

    LabelInfoList_t *labelList =
        (LabelInfoList_t *)calloc(1, sizeof(LabelInfoList_t));

    ASN_SEQUENCE_ADD(&labelList->list, labelItem);

    // ---------------------------------------------
    // 4) MeasurementInfoItem 생성
    // ---------------------------------------------
    MeasurementInfoItem_t *infoItem =
        (MeasurementInfoItem_t *)calloc(1, sizeof(MeasurementInfoItem_t));

    infoItem->measType = *measType;
    infoItem->labelInfoList = *labelList;

    return {infoItem, measDataItem};


  /*
  *measurmentType = item.pmType;

  if (item.pmType.present == MeasurementType_PR_measName)
    {
      switch (item.pmVal.present)
        {
          case MeasurementValue_PR_NOTHING: {
            NS_LOG_INFO ("\n MeasurementValue_PR_NOTHING");
            break;
          }
          case MeasurementValue_PR_valueInt: {
            NS_LOG_INFO ("\n 	MeasurementValue_PR_valueInt" << item.pmVal.choice.valueInt);
            measureDataItem = getMesDataItem (static_cast<double> (item.pmVal.choice.valueInt));
            break;
          }
          case MeasurementValue_PR_valueReal: {
            NS_LOG_INFO ("\n MeasurementValue_PR_valueReal" << item.pmVal.choice.valueReal);
            measureDataItem = getMesDataItem (item.pmVal.choice.valueReal);
            break;
          }
          case MeasurementValue_PR_noValue: {
            NS_LOG_INFO ("\n MeasurementValue_PR_noValue" << item.pmVal.choice.noValue);
            break;
          }
          case MeasurementValue_PR_valueRRC: {
            // LabelInfoList_t *labelInfoolist = (LabelInfoList_t *) calloc (1, sizeof (LabelInfoList_t));

            auto measName =
                std::string (reinterpret_cast<const char *> (item.pmType.choice.measName.buf),
                             item.pmType.choice.measName.size);
            NS_LOG_INFO ("\n 	item.pmType.choice.measName" << measName);

            // Fill Neighbours cells of 5G, MeasResultNeighCells_PR_measResultListNR
            L3_RRC_Measurements *rrc = item.pmVal.choice.valueRRC;

            if (measName == "HO.TrgtCellQual.RS-SINR.UEID")
              {
                if (rrc->measResultNeighCells != nullptr &&
                    rrc->measResultNeighCells->present == MeasResultNeighCells_PR_measResultListNR)
                  {
                    assert (rrc->measResultNeighCells->choice.measResultListNR != nullptr);
                    try
                      {
                        if (rrc->measResultNeighCells->choice.measResultListNR->list.count > 0)
                          {
                            MeasResultListNR *_measResultListNR =
                                rrc->measResultNeighCells->choice.measResultListNR;
                            measureDataItem = getMesDataItem (_measResultListNR);
                          }

                    } catch (const std::exception &e)
                      {
                        std::cerr << "Error is" << e.what () << '\n';
                    }
                  }
              }
            else if (measName == "HO.SrcCellQual.RS-SINR.UEID")
              {
                if (rrc->servingCellMeasurements != nullptr &&
                    rrc->servingCellMeasurements->present ==
                        ServingCellMeasurements_PR_nr_measResultServingMOList)
                  {
                    assert (rrc->servingCellMeasurements->choice.nr_measResultServingMOList !=
                            nullptr);
                    try
                      {
                        if (rrc->servingCellMeasurements->choice.nr_measResultServingMOList->list
                                .count > 0)
                          {
                            MeasResultServMOList *_MeasResultServMO =
                                rrc->servingCellMeasurements->choice.nr_measResultServingMOList;
                            measureDataItem = getMesDataItem (_MeasResultServMO);
                          }
                    } catch (const std::exception &e)
                      {
                        std::cerr << "Error is" << e.what () << '\n';
                    }
                  }
              }
            else
              {
                NS_LOG_INFO ("\n 	L3_RRC_Measurement but not handled, measName" << measName);
              }

            break;
          }
          default: {
            break;
          }
        }
    }
  else
    {
      NS_LOG_INFO ("\n 	item.pmType.choice.measID" << item.pmType.choice.measID);
    }

  MeasurementLabel_t *measure_label =
      (MeasurementLabel_t *) calloc (1, sizeof (MeasurementLabel_t));
  
  measure_label->noLabel = (long *) malloc (sizeof (long));
  assert (measure_label->noLabel != NULL && "Memory exhausted");
  *measure_label->noLabel = 0;
 
  LabelInfoItem_t *LabelInfoItem = (LabelInfoItem_t *) calloc (1, sizeof (LabelInfoItem_t));
  LabelInfoItem->measLabel = *measure_label;
  LabelInfoList_t *labelInfoolist = (LabelInfoList_t *) calloc (1, sizeof (LabelInfoList_t));
  ASN_SEQUENCE_ADD (&labelInfoolist->list, LabelInfoItem);

  MeasurementInfoItem_t *infoItem =
      (MeasurementInfoItem_t *) calloc (1, sizeof (MeasurementInfoItem_t));

  infoItem->labelInfoList = *labelInfoolist;
  infoItem->measType = *measurmentType;

  return std::make_pair (infoItem, measureDataItem);
  */
}

MeasurementDataItem_t *
KpmIndicationMessage::getMesDataItem (const double &realVal)
{

  // 1. Adding a measurement values(item) to the packet.
  MeasurementRecord_t *measure_record =
      (MeasurementRecord_t *) calloc (1, sizeof (MeasurementRecord_t));

  MeasurementRecordItem_t *measure_record_item =
      (MeasurementRecordItem_t *) calloc (1, sizeof (MeasurementRecordItem_t));

  measure_record_item->present = MeasurementRecordItem_PR_real;
  measure_record_item->choice.real = realVal;

  // Stream measurement records to list.
  ASN_SEQUENCE_ADD (&measure_record->list, measure_record_item);

  MeasurementDataItem_t *measure_data_item =
      (MeasurementDataItem_t *) calloc (1, sizeof (MeasurementDataItem_t));

  measure_data_item->measRecord = *measure_record;

  return measure_data_item;
}

MeasurementDataItem_t *
KpmIndicationMessage::getMesDataItem (const MeasResultServMOList *_MeasResultServMOList)
{

  MeasurementRecord_t *measure_record =
      (MeasurementRecord_t *) calloc (1, sizeof (MeasurementRecord_t));

  MeasurementDataItem_t *measure_data_item =
      (MeasurementDataItem_t *) calloc (1, sizeof (MeasurementDataItem_t));

  for (int i = 0; i < _MeasResultServMOList->list.count; i++)
    {
      MeasurementRecordItem_t *measure_record_item1 =
          (MeasurementRecordItem_t *) calloc (1, sizeof (MeasurementRecordItem_t));
      measure_record_item1->present = MeasurementRecordItem_PR_integer;
      measure_record_item1->choice.integer =
          static_cast<unsigned long> (_MeasResultServMOList->list.array[i]->servCellId);

      MeasurementRecordItem_t *measure_record_item2 =
          (MeasurementRecordItem_t *) calloc (1, sizeof (MeasurementRecordItem_t));
      measure_record_item2->present = MeasurementRecordItem_PR_integer;
      measure_record_item2->choice.integer = static_cast<unsigned long> (
          *(_MeasResultServMOList->list.array[i]->measResultServingCell.physCellId));

      MeasurementRecordItem_t *measure_record_item3 =
          (MeasurementRecordItem_t *) calloc (1, sizeof (MeasurementRecordItem_t));
      measure_record_item3->present = MeasurementRecordItem_PR_real;
      measure_record_item3->choice.real =
          *(_MeasResultServMOList->list.array[i]
                ->measResultServingCell.measResult.cellResults.resultsSSB_Cell->sinr);

      // Stream measurement records to list.
      ASN_SEQUENCE_ADD (&measure_record->list, measure_record_item1);
      ASN_SEQUENCE_ADD (&measure_record->list, measure_record_item2);
      ASN_SEQUENCE_ADD (&measure_record->list, measure_record_item3);
    }

  measure_data_item->measRecord = *measure_record;

  return measure_data_item;
}

MeasurementDataItem_t *
KpmIndicationMessage::getMesDataItem (const MeasResultListNR *_measResultListNR)
{

  MeasurementRecord_t *measure_record =
      (MeasurementRecord_t *) calloc (1, sizeof (MeasurementRecord_t));

  MeasurementDataItem_t *measure_data_item =
      (MeasurementDataItem_t *) calloc (1, sizeof (MeasurementDataItem_t));

  for (int i = 0; i < _measResultListNR->list.count; i++)
    {
      MeasurementRecordItem_t *measure_record_item2 =
          (MeasurementRecordItem_t *) calloc (1, sizeof (MeasurementRecordItem_t));
      measure_record_item2->present = MeasurementRecordItem_PR_integer;
      measure_record_item2->choice.integer = *(_measResultListNR->list.array[i]->physCellId);

      MeasurementRecordItem_t *measure_record_item =
          (MeasurementRecordItem_t *) calloc (1, sizeof (MeasurementRecordItem_t));

      measure_record_item->present = MeasurementRecordItem_PR_real;
      measure_record_item->choice.real =
          *(_measResultListNR->list.array[i]
                ->measResult.cellResults.resultsSSB_Cell->sinr); //(rand() % 256) + 0.1;

      // Stream measurement records to list.
      ASN_SEQUENCE_ADD (&measure_record->list, measure_record_item2);
      ASN_SEQUENCE_ADD (&measure_record->list, measure_record_item);
    }

  measure_data_item->measRecord = *measure_record;

  return measure_data_item;
}

// E-UTRA is the air interface of 3rd Generation Partnership Project (3GPP)
// Long Term Evolution (LTE) upgrade path for mobile networks.

// ueVal->AddItem<Ptr<L3RrcMeasurements>> ("HO.SrcCellQual.RS-SINR.UEID", l3RrcMeasurementServing);
// ueVal->AddItem<Ptr<L3RrcMeasurements>> ("HO.TrgtCellQual.RS-SINR.UEID", l3RrcMeasurementNeigh);
// RIC Style 4 cellID, UserID, SINR Map
// Event trigger - PrbUsage
// 0.371 enbdev 2 UE 3 L3 neigh 3 SINR -3.92736 sinr encoded 38 first insert
// 0.371 enbdev 2 UE 3 L3 neigh 4 SINR -11.9135 sinr encoded 22 first insert
// 0.371 enbdev 2 UE 4 L3 serving SINR -13.7703 L3 serving SINR 3gpp 19
// 0.371 enbdev 2 UE 4 L3 neigh 4 SINR -10.8886 sinr encoded 24 first insert
// 0.371 enbdev 2 UE 4 L3 neigh 3 SINR -34.2883 sinr encoded 0 first insert
// 0.371 enbdev 2 UE 1 L3 serving SINR -0.616781 L3 serving SINR 3gpp 45
// 0.371 enbdev 2 UE 1 L3 neigh 3 SINR -9.88701 sinr encoded 26 first insert

std::string
servingMsg (const int &cellID, const int &UeID)
{
  // 0.371 enbdev 2 UE 3 L3 serving SINR 3.28529 L3 serving SINR 3gpp 53
  std::ostringstream oss;
  // L3servingSINR3gpp_cell_XX
  // L3servingSINR3gpp_cell_XX_UEID_XX
  oss << "L3servingSINR3gpp_cell_" << cellID << "_UEID_" << UeID;
  return oss.str ();
}

std::string
neighMsg (const int &cellID)
{
  // 0.371 enbdev 2 UE 1 L3 neigh 4 SINR -19.4777 sinr encoded 7 first insert
  std::ostringstream oss;
  // L3neighSINR_cell_XX
  oss << "L3neighSINR_cell_" << cellID;
  // return std::move (oss.str ());
  return oss.str ();
}


// ==============================================
// Indication Based Make Format 1 Message
// ==============================================
void
KpmIndicationMessage::FillKpmIndicationMessageFormat1(
    E2SM_KPM_IndicationMessage_Format1_t *format,
    Ptr<MeasurementItemList> indication,
    const std::string& cellObjectId /* = "" */)
{

  NS_LOG_FUNCTION(this << format << indication << cellObjectId);

  MeasurementData_t *measData = &format->measData;

  // 1) MeasurementInfoList: KPI 정의 목록 (KPI 개수 = items.size())
  MeasurementInfoList_t *infoList =
      (MeasurementInfoList_t *)calloc(1, sizeof(*infoList));

  auto items = indication->GetItems();
  NS_LOG_DEBUG("FillKpmIndicationMessageFormat1(): items=" << items.size()
                << " cellObjectId=" << cellObjectId);
    for (const auto &it : items)
    {
      auto v = it->GetValue();  // pmType, pmVal 등

      // measType = measName 기반으로 설정
      if (v.pmType.present != MeasurementType_PR_measName)
        {
          NS_LOG_WARN("KPM: pmType is not measName, skip this item");
          continue;
        }

      MeasurementInfoItem_t *infoItem =
          (MeasurementInfoItem_t *)calloc(1, sizeof(*infoItem));

      // MeasurementType(measName)
      MeasurementType_t *measType =
          (MeasurementType_t *)calloc(1, sizeof(*measType));
      measType->present = MeasurementType_PR_measName;

      std::string measName(
          (char *)v.pmType.choice.measName.buf,
          v.pmType.choice.measName.size);

      OCTET_STRING_fromBuf(&measType->choice.measName,
                           measName.c_str(),
                           measName.size());

      infoItem->measType = *measType;

      // LabelInfoList: 셀 공통 KPI라면 noLabel만 사용
      MeasurementLabel_t *measLabel =
          (MeasurementLabel_t *)calloc(1, sizeof(*measLabel));
      measLabel->noLabel = (long *)calloc(1, sizeof(long));
      *measLabel->noLabel = 0;

      LabelInfoItem_t *labelItem =
          (LabelInfoItem_t *)calloc(1, sizeof(*labelItem));
      labelItem->measLabel = *measLabel;

      LabelInfoList_t *labelList =
          (LabelInfoList_t *)calloc(1, sizeof(*labelList));
      ASN_SEQUENCE_ADD(&labelList->list, labelItem);

      infoItem->labelInfoList = *labelList;

      ASN_SEQUENCE_ADD(&infoList->list, infoItem);

      NS_LOG_DEBUG("FillKpmIndicationMessageFormat1(): item Name=" << measName);
    }

  // KPI 정의가 하나도 없으면 그냥 종료
  if (infoList->list.count == 0)
    {
      NS_LOG_WARN("KPM: no MeasurementInfoItem built, skip IndicationMessage");
      ASN_STRUCT_FREE(asn_DEF_MeasurementInfoList, infoList);
      return;
    }

  format->measInfoList = infoList;

  // 2) MeasurementData: 셀마다 1개의 MeasurementDataItem
  //    지금은 셀 1개 기준 → dataItem 1개만 생성
  MeasurementDataItem_t *dataItem =
      (MeasurementDataItem_t *)calloc(1, sizeof(*dataItem));

  // === 2-1. 이 셀의 KPI 값들을 MeasurementRecord로 채움 ===
  for (const auto &it : items)
    {
      auto v = it->GetValue();
      MeasurementRecordItem_t *rec =
    (MeasurementRecordItem_t*)calloc(1, sizeof(*rec));

      switch (v.pmVal.present)
        {
        case MeasurementValue_PR_valueInt:
        rec->present = MeasurementRecordItem_PR_integer;
        rec->choice.integer = v.pmVal.choice.valueInt;
        break;

    case MeasurementValue_PR_valueReal:
        rec->present = MeasurementRecordItem_PR_real;
        rec->choice.real = v.pmVal.choice.valueReal;
        break;

    case MeasurementValue_PR_noValue:
    default:
        rec->present = MeasurementRecordItem_PR_noValue;
        // choice 없음
        break;
    }

      ASN_SEQUENCE_ADD(&dataItem->measRecord.list, rec);
    }

  // dataItem 하나 = "이 셀의 모든 KPI 값"
  ASN_SEQUENCE_ADD(&measData->list, dataItem);

  // 3) GranularityPeriod 설정
  GranularityPeriod_t *gran =
      (GranularityPeriod_t *)calloc(1, sizeof(*gran));
  *gran = 100;  // ms 단위 등, 네 환경에 맞게 조정

  format->granulPeriod = gran;

  NS_LOG_DEBUG("FillKpmIndicationMessageFormat1(): "
               << "measInfo(cnt)=" << format->measInfoList->list.count
               << " measData(cnt)=" << measData->list.count
               << " (expected: cells=1, kpis=" << items.size() << ")");
}

void
KpmIndicationMessage::FillKpmIndicationMessageFormat3(
    E2SM_KPM_IndicationMessage_Format3 *fmt3,
    const KpmIndicationMessageValues &values)
{
  NS_LOG_DEBUG("FillKpmIndicationMessageFormat3(): start, UEs=" << values.m_ueIndications.size());
  for (const auto &ueIndication : values.m_ueIndications)
  {
    Ptr<MeasurementItemList> ueList = ueIndication;
    if (!ueList) {
      NS_LOG_WARN("FillKpmIndicationMessageFormat3(): null UE indication → creating dummy");
      ueList = Create<MeasurementItemList>("NO_UE_ID");
      ueList->AddItem("No Data", 0.0);
    }
    
    UEMeasurementReportItem_t *UE_data = (UEMeasurementReportItem_t*) calloc(1, sizeof(*UE_data));
    if (!UE_data) {
      NS_FATAL_ERROR("calloc failed for UE_data");
    }
    FillUeID(&UE_data->ueID, ueIndication);

    // JLEE: For handle empty ueIndiation item by injecting dummy item 
    if (ueList->GetItems().empty()) {
      NS_LOG_DEBUG("FillKpmIndicationMessageFormat3(): UE has no measurement items → using dummy Format1");
      ueList->AddItem("DRB.PdcpSduVolumeDl_Filter.UEID", 0.0);
      ueList->AddItem("Tot.PdcpSduNbrDl.UEID", 0.1);
      ueList->AddItem("DRB.PdcpSduBitRateDl.UEID", 0.2);
      ueList->AddItem("DRB.PdcpSduDelayDl.UEID", 0.3);
      ueList->AddItem("DRB.EstabSucc.5QI.UEID", 0.4);
      ueList->AddItem("DRB.RelActNbr.5QI.UEID", 0.5);

    }
    FillKpmIndicationMessageFormat1(&UE_data->measReport, ueList);

    ASN_SEQUENCE_ADD(&fmt3->ueMeasReportList.list, UE_data);
  }
  NS_LOG_DEBUG("FillKpmIndicationMessageFormat3(): done, UEs=" << values.m_ueIndications.size());
}



std::vector<UeReport>
KpmIndicationMessage::ExtractUeReports(const KpmIndicationMessageValues &values)
{
    std::vector<UeReport> reports;

    for (const auto &ueList : values.m_ueIndications)
    {
        UeReport rep;
        auto items = ueList->GetItems();

        for (auto& item : items)
        {
            auto v = item->GetValue();
            std::string measName(
                (char *)v.pmType.choice.measName.buf,
                v.pmType.choice.measName.size);
            rep.metricNames.push_back(measName);

            switch (v.pmVal.present)
            {
                case MeasurementValue_PR_valueInt:
                    rep.metricValues.push_back(v.pmVal.choice.valueInt);
                    break;
                case MeasurementValue_PR_valueReal:
                    rep.metricValues.push_back(v.pmVal.choice.valueReal);
                    break;
                default:
                    break;
            }
        }

        // ❗ UE 하나당 Rep 하나만 push_back
        reports.push_back(rep);
    }

    return reports;
}
//

void
KpmIndicationMessage::FillKpmIndicationMessageFormat2(
    E2SM_KPM_IndicationMessage_Format2 *fmt2,
    const KpmIndicationMessageValues &values)
{
  NS_LOG_DEBUG("FillKpmIndicationMessageFormat2(): start, UEs="
               << values.m_ueIndications.size());

  auto ueReports = ExtractUeReports(values);

  std::vector<Ptr<MeasurementItemList>> ueInds(
      values.m_ueIndications.begin(),
      values.m_ueIndications.end());
  

  int ueCount = ueReports.size();
  if (ueCount == 0)
    {
      NS_LOG_WARN("Format2: no UE reports");
      return;
    }

  int metricCount = ueReports[0].metricNames.size();
  if (metricCount == 0)
    {
      NS_LOG_WARN("Format2: no metrics");
      return;
    }


  fmt2->measData.list.count = metricCount;
  fmt2->measData.list.size  = metricCount;
  fmt2->measData.list.array =
      (MeasurementDataItem_t **)calloc(metricCount, sizeof(MeasurementDataItem_t *));

  for (int m = 0; m < metricCount; ++m)
    {
      MeasurementDataItem_t *dataItem =
          (MeasurementDataItem_t *)calloc(1, sizeof(MeasurementDataItem_t));

      dataItem->measRecord.list.count = ueCount;
      dataItem->measRecord.list.size  = ueCount;
      dataItem->measRecord.list.array =
          (MeasurementRecordItem_t **)calloc(ueCount, sizeof(MeasurementRecordItem_t *));

      for (int u = 0; u < ueCount; ++u)
        {
          MeasurementRecordItem_t *rec =
              (MeasurementRecordItem_t *)calloc(1, sizeof(MeasurementRecordItem_t));

          rec->present     = MeasurementRecordItem_PR_real;
          rec->choice.real = ueReports[u].metricValues[m];

          dataItem->measRecord.list.array[u] = rec;
        }

      fmt2->measData.list.array[m] = dataItem;
    }

  ///
    MeasurementCondUEidList_t *condList = &fmt2->measCondUEidList;


    condList->list.count = 1;
    condList->list.size  = 1;
    condList->list.array =
        (MeasurementCondUEidItem_t **)calloc(1, sizeof(MeasurementCondUEidItem_t *));

    MeasurementCondUEidItem_t *item =
        (MeasurementCondUEidItem_t*)calloc(1, sizeof(MeasurementCondUEidItem_t));
    condList->list.array[0] = item;
//------------------------------
// 1. MeasurementType 설정
//------------------------------
  item->measType.present = MeasurementType_PR_measName;

  const std::string &firstMetricName = ueReports[0].metricNames[0];
  OCTET_STRING_fromString(&item->measType.choice.measName,
                          firstMetricName.c_str());

//------------------------------
// 2. MatchingCondList 설정
//------------------------------
  MatchingCondList_t *mcl = &item->matchingCond;
  mcl->list.count = 1;
  mcl->list.size  = 1;
  mcl->list.array =
      (MatchingCondItem_t **)calloc(1, sizeof(MatchingCondItem_t *));

  MatchingCondItem_t *mc =
      (MatchingCondItem_t *)calloc(1, sizeof(MatchingCondItem_t));
  mcl->list.array[0] = mc;

  mc->matchingCondChoice.present = MatchingCondItem_Choice_PR_measLabel;
  mc->matchingCondChoice.choice.measLabel =
      (MeasurementLabel_t *)calloc(1, sizeof(MeasurementLabel_t));

  mc->logicalOR = NULL;  // OPTIONAL

  item->matchingUEidList =
        (MatchingUEidList_t *)calloc(1, sizeof(MatchingUEidList_t));

    MatchingUEidList_t *ueidList = item->matchingUEidList;

    ueidList->list.count = ueCount;
    ueidList->list.size  = ueCount;
    ueidList->list.array =
        (MatchingUEidItem_t **)calloc(ueCount,
                                      sizeof(MatchingUEidItem_t *));

    for (int u = 0; u < ueCount; ++u)
      {
        MatchingUEidItem_t *ueItem =
            (MatchingUEidItem_t *)calloc(1, sizeof(MatchingUEidItem_t));

        // UEID_t 한 개를 FillUeID로 채운다.
        // values.m_ueIndications[u] 가 Ptr<MeasurementItemList> 라고 가정
        FillUeID(&ueItem->ueID, ueInds[u]);

        ueidList->list.array[u] = ueItem;

      }

    // matchingUEidPerGP 는 OPTIONAL
    item->matchingUEidPerGP = NULL;


  // ----------------------------------------------------
  // (2) Granularity Period (optional)
  // ----------------------------------------------------
  auto *gran = (GranularityPeriod_t *)calloc(1, sizeof(GranularityPeriod_t));
  *gran      = 100;   // 100 ms (환경에 맞게 바꿔도 됨)

  fmt2->granulPeriod = gran;

  NS_LOG_DEBUG("FillKpmIndicationMessageFormat2(): done");
}


static inline void os_to_ran_ueid_8bytes(const OCTET_STRING_t& os, uint8_t out[8]) {
  const uint64_t FNV_OFFSET = 1469598103934665603ULL;
  const uint64_t FNV_PRIME  = 1099511628211ULL;
  uint64_t h = FNV_OFFSET;
  for (size_t i = 0; i < os.size; ++i) {
    h ^= (uint8_t)os.buf[i];
    h *= FNV_PRIME;
  }
  memcpy(out, &h, 8);
}



void
KpmIndicationMessage::FillUeID (UEID_t *ue_ID, const Ptr<MeasurementItemList> ueIndication)
{

  UEID_GNB_t *gnb_asn = (UEID_GNB_t*)calloc(1, sizeof(*gnb_asn));
  unsigned long amf_id = (unsigned long) KpmIndicationHeader::octet_string_to_int_64(ueIndication->GetId());   // 여러분의 UE ID를 정수로
  asn_ulong2INTEGER(&gnb_asn->amf_UE_NGAP_ID, amf_id);

  gnb_asn->guami.aMFPointer  = cp_amf_ptr_to_bit_string((rand() % (1<<6)));
  gnb_asn->guami.aMFSetID    = cp_amf_set_id_to_bit_string((rand() % (1<<10)));
  gnb_asn->guami.aMFRegionID = cp_amf_region_id_to_bit_string((rand() % (1<<6))); //fixed under 64 by jlee
  gnb_asn->guami.pLMNIdentity= cp_plmn_identity_to_octant_string(rand()%505, rand()%99, 2);

  gnb_asn->ran_UEID = (RANUEID_t*)calloc(1, sizeof(*gnb_asn->ran_UEID));
  gnb_asn->ran_UEID->buf  = (uint8_t*)calloc(8, 1);
  gnb_asn->ran_UEID->size = 8;
  os_to_ran_ueid_8bytes(ueIndication->GetId(), gnb_asn->ran_UEID->buf);

  ue_ID->present        = UEID_PR_gNB_UEID;
  ue_ID->choice.gNB_UEID= gnb_asn;
}

void
KpmIndicationMessage::FillAndEncodeKpmIndicationMessage (
    E2SM_KPM_IndicationMessage_t *descriptor, KpmIndicationMessageValues values,
    const E2SM_KPM_IndicationMessage_FormatType &format_type)
{

  ASN_STRUCT_RESET(asn_DEF_E2SM_KPM_IndicationMessage, descriptor);

  switch (format_type)
    {
      case E2SM_KPM_INDICATION_MESSAGE_FORMART1: {
        NS_LOG_DEBUG("Encode E2SM_KPM_I For Cell");
        
        E2SM_KPM_IndicationMessage_Format1_t *msg_fmt1 =
            (E2SM_KPM_IndicationMessage_Format1_t *)calloc(1, sizeof(*msg_fmt1));
        if (!msg_fmt1) {
            NS_FATAL_ERROR("calloc failed for msg_fmt1");
        }
        // Check Object Cell ID
        const std::string cellId = values.m_cellObjectId.empty()
                               ? "NO_CELL_ID" : values.m_cellObjectId;


        Ptr<MeasurementItemList> cellItems = values.m_cellMeasurementItems;
        if (!cellItems)
        {
            NS_LOG_DEBUG("Creating MeasurementItemList For Cell");
            cellItems = Create<MeasurementItemList>(cellId);
            cellItems->AddItem("DRB.PdcpSduDelayDl", 0.0);
            cellItems->AddItem("pdcpBytesUl", 0.1);
            cellItems->AddItem("pdcpBytesDl", 0.2);
            cellItems->AddItem("numActiveUes", 0.3);
        }

        FillKpmIndicationMessageFormat1(msg_fmt1, cellItems, cellId);
        
        descriptor->indicationMessage_formats.present =
            E2SM_KPM_IndicationMessage__indicationMessage_formats_PR_indicationMessage_Format1;
        descriptor->indicationMessage_formats.choice.indicationMessage_Format1 = msg_fmt1;
        break;
      }
      case E2SM_KPM_INDICATION_MESSAGE_FORMART2: {
          E2SM_KPM_IndicationMessage_Format2_t *fmt2 = (E2SM_KPM_IndicationMessage_Format2_t*)calloc(1, sizeof(*fmt2));

          if (!fmt2) { NS_FATAL_ERROR("calloc failed for E2SM_KPM_IndicationMessage_Format2_t");}

          if (!values.m_ueIndications.empty()) {
              FillKpmIndicationMessageFormat2 (fmt2, values);
          } else {
        }

        descriptor->indicationMessage_formats.present =
            E2SM_KPM_IndicationMessage__indicationMessage_formats_PR_indicationMessage_Format2;
        descriptor->indicationMessage_formats.choice.indicationMessage_Format2 = fmt2;
        break;


      }
      case E2SM_KPM_INDICATION_MESSAGE_FORMART3: {
          NS_LOG_DEBUG("Encode E2SM_KPM_I For UE");

          E2SM_KPM_IndicationMessage_Format3_t *fmt3 = (E2SM_KPM_IndicationMessage_Format3_t*)calloc(1, sizeof(*fmt3));

          if (!fmt3) { NS_FATAL_ERROR("calloc failed for E2SM_KPM_IndicationMessage_Format3_t");}

          if (!values.m_ueIndications.empty()) {
              FillKpmIndicationMessageFormat3 (fmt3, values);
          } else {
          // create dumy indication message
          UEMeasurementReportItem_t *UE_data = (UEMeasurementReportItem_t*)calloc(1, sizeof(*UE_data));
          UEID_GNB_t *gnb_asn = (UEID_GNB_t*)calloc(1, sizeof(*gnb_asn));
          gnb_asn->amf_UE_NGAP_ID.buf = (uint8_t*)calloc(5, sizeof(uint8_t));
          asn_ulong2INTEGER(&gnb_asn->amf_UE_NGAP_ID, 1);
          gnb_asn->ran_UEID = (RANUEID_t*)calloc(1, sizeof(*gnb_asn->ran_UEID));
          gnb_asn->ran_UEID->buf  = (uint8_t*)calloc(8, sizeof(uint8_t));
          gnb_asn->ran_UEID->size = 8;
          UE_data->ueID.present      = UEID_PR_gNB_UEID;
          UE_data->ueID.choice.gNB_UEID = gnb_asn;
          
          Ptr<MeasurementItemList> dummyUeList = Create<MeasurementItemList>("UE_DUMMY_ID");
          dummyUeList->AddItem("DRB.PdcpSduVolumeDl_Filter.UEID", 0.0);
          dummyUeList->AddItem("Tot.PdcpSduNbrDl.UEID", 0.0);
          dummyUeList->AddItem("DRB.PdcpSduBitRateDl.UEID", 0.0);
          dummyUeList->AddItem("DRB.PdcpSduDelayDl.UEID", 0.0);
          dummyUeList->AddItem("RB.EstabSucc.5QI.UEID", 0.0);
          dummyUeList->AddItem("DRB.RelActNbr.5QI.UEID", 0.0);
          FillKpmIndicationMessageFormat1(&UE_data->measReport, dummyUeList); // To be update from 1028
          ASN_SEQUENCE_ADD(&fmt3->ueMeasReportList.list, UE_data);
        }

        descriptor->indicationMessage_formats.present =
            E2SM_KPM_IndicationMessage__indicationMessage_formats_PR_indicationMessage_Format3;
        descriptor->indicationMessage_formats.choice.indicationMessage_Format3 = fmt3;
        break;
      }
    default:
      break;
    }
  NS_LOG_DEBUG(">>> present = " << descriptor->indicationMessage_formats.present);
  NS_LOG_DEBUG(">>> fmt3 ptr = " << (void*)descriptor->indicationMessage_formats.choice.indicationMessage_Format3);
  NS_LOG_DEBUG(">>> ueMeasReportList count = "
    << descriptor->indicationMessage_formats.choice.indicationMessage_Format3->ueMeasReportList.list.count);

  //NS_LOG_DEBUG (xer_fprint (stderr, &asn_DEF_E2SM_KPM_IndicationMessage, descriptor));
  Encode (descriptor);
  printf (" \n *** Done Encoding INDICATION Message ****** \n ");
}


MeasurementItemList::MeasurementItemList ()
{
  m_id = NULL;
}

MeasurementItemList::MeasurementItemList (std::string id)
{
  m_id = Create<OctetString> (id, id.length ());
}

MeasurementItemList::~MeasurementItemList (){};

std::vector<Ptr<MeasurementItem>>
MeasurementItemList::GetItems ()
{
  return m_items;
}

OCTET_STRING_t
MeasurementItemList::GetId ()
{
  NS_ABORT_IF (m_id == NULL);
  return m_id->GetValue ();
}

// To be deleted the blow function
void
KpmIndicationMessage::FillPmContainer (PF_Container_t *ranContainer, Ptr<PmContainerValues> values)
{
  Ptr<OCuUpContainerValues> cuUpVal = DynamicCast<OCuUpContainerValues> (values);
  Ptr<OCuCpContainerValues> cuCpVal = DynamicCast<OCuCpContainerValues> (values);
  Ptr<ODuContainerValues> duVal = DynamicCast<ODuContainerValues> (values);

  if (cuUpVal)
    {
      FillOCuUpContainer (ranContainer, cuUpVal);
    }
  else if (cuCpVal)
    {
      FillOCuCpContainer (ranContainer, cuCpVal);
    }
  else if (duVal)
    {
      FillODuContainer (ranContainer, duVal);
    }
  else
    {
      NS_FATAL_ERROR ("Unknown PM Container type");
    }
}
// To be deleted
void
KpmIndicationMessage::FillOCuUpContainer (PF_Container_t *ranContainer,
                                          Ptr<OCuUpContainerValues> values)
{
  OCUUP_PF_Container_t *ocuup = (OCUUP_PF_Container_t *) calloc (1, sizeof (OCUUP_PF_Container_t));
  PF_ContainerListItem_t *pcli =
      (PF_ContainerListItem_t *) calloc (1, sizeof (PF_ContainerListItem_t));
  pcli->interface_type = NI_Type_x2_u;

  CUUPMeasurement_Container_t *cuuppmc =
      (CUUPMeasurement_Container_t *) calloc (1, sizeof (CUUPMeasurement_Container_t));
  PlmnID_Item_t *plmnItem = (PlmnID_Item_t *) calloc (1, sizeof (PlmnID_Item_t));
  Ptr<OctetString> plmnidstr = Create<OctetString> (values->m_plmId, 3);
  plmnItem->pLMN_Identity = plmnidstr->GetValue ();

  EPC_CUUP_PM_Format_t *cuuppmf =
      (EPC_CUUP_PM_Format_t *) calloc (1, sizeof (EPC_CUUP_PM_Format_t));
  plmnItem->cu_UP_PM_EPC = cuuppmf;
  PerQCIReportListItemFormat_t *pqrli =
      (PerQCIReportListItemFormat_t *) calloc (1, sizeof (PerQCIReportListItemFormat_t));
  pqrli->drbqci = 0;

  INTEGER_t *pDCPBytesDL = (INTEGER_t *) calloc (1, sizeof (INTEGER_t));
  INTEGER_t *pDCPBytesUL = (INTEGER_t *) calloc (1, sizeof (INTEGER_t));

  asn_long2INTEGER (pDCPBytesDL, values->m_pDCPBytesDL);
  asn_long2INTEGER (pDCPBytesUL, values->m_pDCPBytesUL);

  pqrli->pDCPBytesDL = pDCPBytesDL;
  pqrli->pDCPBytesUL = pDCPBytesUL;

  ASN_SEQUENCE_ADD (&cuuppmf->perQCIReportList_cuup.list, pqrli);

  ASN_SEQUENCE_ADD (&cuuppmc->plmnList.list, plmnItem);

  pcli->o_CU_UP_PM_Container = *cuuppmc;
  ASN_SEQUENCE_ADD (&ocuup->pf_ContainerList, pcli);
  ranContainer->choice.oCU_UP = ocuup;
  ranContainer->present = PF_Container_PR_oCU_UP;

  free (cuuppmc);
}
// To be deleted

void
KpmIndicationMessage::FillOCuCpContainer (PF_Container_t *ranContainer,
                                          Ptr<OCuCpContainerValues> values)
{
  OCUCP_PF_Container_t *ocucp = (OCUCP_PF_Container_t *) calloc (1, sizeof (OCUCP_PF_Container_t));
  long *numActiveUes = (long *) calloc (1, sizeof (long));
  *numActiveUes = long (values->m_numActiveUes);
  ocucp->cu_CP_Resource_Status.numberOfActive_UEs = numActiveUes;
  ranContainer->choice.oCU_CP = ocucp;
  ranContainer->present = PF_Container_PR_oCU_CP;
}
// To be deleted

void
KpmIndicationMessage::FillODuContainer (PF_Container_t *ranContainer,
                                        Ptr<ODuContainerValues> values)
{
  ODU_PF_Container_t *odu = (ODU_PF_Container_t *) calloc (1, sizeof (ODU_PF_Container_t));

  for (auto cellReport : values->m_cellResourceReportItems)
    {
      NS_LOG_LOGIC ("O-DU: Add Cell Resource Report Item");
      CellResourceReportListItem_t *crrli =
          (CellResourceReportListItem_t *) calloc (1, sizeof (CellResourceReportListItem_t));

      Ptr<OctetString> plmnid = Create<OctetString> (cellReport->m_plmId, 3);
      Ptr<NrCellId> nrcellid = Create<NrCellId> (cellReport->m_nrCellId);
      crrli->nRCGI.pLMN_Identity = plmnid->GetValue ();
      crrli->nRCGI.nRCellIdentity = nrcellid->GetValue ();

      long *dlAvailablePrbs = (long *) calloc (1, sizeof (long));
      *dlAvailablePrbs = cellReport->dlAvailablePrbs;
      crrli->dl_TotalofAvailablePRBs = dlAvailablePrbs;

      long *ulAvailablePrbs = (long *) calloc (1, sizeof (long));
      *ulAvailablePrbs = cellReport->ulAvailablePrbs;
      crrli->ul_TotalofAvailablePRBs = ulAvailablePrbs;
      ASN_SEQUENCE_ADD (&odu->cellResourceReportList.list, crrli);

      for (auto servedPlmnCell : cellReport->m_servedPlmnPerCellItems)
        {
          NS_LOG_LOGIC ("O-DU: Add Served Plmn Per Cell Item");
          ServedPlmnPerCellListItem_t *sppcl =
              (ServedPlmnPerCellListItem_t *) calloc (1, sizeof (ServedPlmnPerCellListItem_t));
          Ptr<OctetString> servedPlmnId = Create<OctetString> (servedPlmnCell->m_plmId, 3);
          sppcl->pLMN_Identity = servedPlmnId->GetValue ();

          EPC_DU_PM_Container_t *edpc =
              (EPC_DU_PM_Container_t *) calloc (1, sizeof (EPC_DU_PM_Container_t));

          for (auto perQciReportItem : servedPlmnCell->m_perQciReportItems)
            {
              NS_LOG_LOGIC ("O-DU: Add Per QCI Report Item");
              PerQCIReportListItem_t *pqrl =
                  (PerQCIReportListItem_t *) calloc (1, sizeof (PerQCIReportListItem_t));
              pqrl->qci = perQciReportItem->m_qci;

              NS_ABORT_MSG_IF ((perQciReportItem->m_dlPrbUsage < 0) |
                                   (perQciReportItem->m_dlPrbUsage > 100),
                               "As per ASN definition, dl_PRBUsage should be between 0 and 100");
              long *dlUsedPrbs = (long *) calloc (1, sizeof (long));
              *dlUsedPrbs = perQciReportItem->m_dlPrbUsage;
              pqrl->dl_PRBUsage = dlUsedPrbs;
              NS_LOG_LOGIC ("DL PRBs " << *dlUsedPrbs);

              NS_ABORT_MSG_IF ((perQciReportItem->m_ulPrbUsage < 0) |
                                   (perQciReportItem->m_ulPrbUsage > 100),
                               "As per ASN definition, ul_PRBUsage should be between 0 and 100");
              long *ulUsedPrbs = (long *) calloc (1, sizeof (long));
              *ulUsedPrbs = perQciReportItem->m_ulPrbUsage;
              pqrl->ul_PRBUsage = ulUsedPrbs;
              ASN_SEQUENCE_ADD (&edpc->perQCIReportList_du.list, pqrl);
            }

          sppcl->du_PM_EPC = edpc;
          ASN_SEQUENCE_ADD (&crrli->servedPlmnPerCellList.list, sppcl);
        }
    }
  ranContainer->choice.oDU = odu;
  ranContainer->present = PF_Container_PR_oDU;
}

} // namespace ns3
