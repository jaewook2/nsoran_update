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
#include <chrono>


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

//add for maintain meas information
MeasurementItem::MeasurementItem(std::string name, long value)
{
  m_recordItem = nullptr;
  m_infoItem = nullptr;

  CreateInfoItem(name);
  CreateRecordItem(value);
}

MeasurementItem::MeasurementItem(std::string name, double value)
{
  m_recordItem = nullptr;
  m_infoItem = nullptr;

  CreateInfoItem(name);
  CreateRecordItem(value);
}


MeasurementItem::~MeasurementItem()
{
  if (m_recordItem)
  {
    ASN_STRUCT_FREE(asn_DEF_MeasurementRecordItem, m_recordItem);
  }

  if (m_infoItem)
  {
    ASN_STRUCT_FREE(asn_DEF_MeasurementInfoItem, m_infoItem);
  }
}

MeasurementRecordItem_t* 
MeasurementItem::GetRecordItem()
{
  return m_recordItem;
}

MeasurementInfoItem_t* 
MeasurementItem::GetInfoItem()
{
  return m_infoItem;
}

void
MeasurementItem::CreateRecordItem(long value)
{
  m_recordItem = (MeasurementRecordItem_t*)calloc(1, sizeof(MeasurementRecordItem_t));

  m_recordItem->present = MeasurementRecordItem_PR_integer;
  m_recordItem->choice.integer = value;
}

void 
MeasurementItem::CreateRecordItem(double value)
{
  m_recordItem = (MeasurementRecordItem_t*)calloc(1, sizeof(MeasurementRecordItem_t));

  m_recordItem->present = MeasurementRecordItem_PR_real;
  m_recordItem->choice.real = value;
}


void 
MeasurementItem::CreateInfoItem(std::string name)
{
  m_infoItem = (MeasurementInfoItem_t *)calloc(1, sizeof(MeasurementInfoItem_t));

  // ① measType 채우기
  m_infoItem->measType.present = MeasurementType_PR_measName;
  OCTET_STRING_fromString(&m_infoItem->measType.choice.measName, name.c_str());

  // ② labelInfoList(noLabel) 같은 것도 여기서 같이 세팅
  LabelInfoItem_t *labelItem =
      (LabelInfoItem_t *)calloc(1, sizeof(LabelInfoItem_t));

  MeasurementLabel_t *label =
      (MeasurementLabel_t *)calloc(1, sizeof(MeasurementLabel_t));

  label->noLabel = (long *)calloc(1, sizeof(long));
  *label->noLabel = 0;

  labelItem->measLabel = *label;
  ASN_SEQUENCE_ADD(&m_infoItem->labelInfoList.list, labelItem);
}

//=============================================
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

  // descriptor는 여기서 굳이 memset 안 해도 됨
  // (밖에서 0으로 초기화되어 있다고 가정하면 생략 가능)
  // memset(descriptor, 0, sizeof(*descriptor));

  E2SM_KPM_IndicationHeader_Format1_t* ind_header =
    (E2SM_KPM_IndicationHeader_Format1_t*)calloc(1, sizeof(*ind_header));

  // ---- 1) host_time: us 단위로 들어온다고 가정 ----
  uint64_t host_time = values.m_timestamp;

  if (host_time == 0)
    {
      using namespace std::chrono;
      host_time =
          std::chrono::duration_cast<std::chrono::microseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count();
    }

  // ---- 2) 32비트로 줄이기 (예: 초 단위) ----
  uint32_t ts32 = (uint32_t)(host_time / 1000000ULL); // us -> sec

#if defined(_WIN32)
  uint32_t net_ts32 = _byteswap_ulong(ts32);
#else
  uint32_t net_ts32 = htobe32(ts32);
#endif

  // ---- 3) TimeStamp (SIZE(4)) 채우기 ----
  ind_header->colletStartTime.buf  = (uint8_t*)calloc(4, 1);
  ind_header->colletStartTime.size = 4;
  memcpy(ind_header->colletStartTime.buf, &net_ts32, 4);

  NS_LOG_INFO("colletStartTime host(us)=" << host_time
                << " sec=" << ts32);

  // ---- 4) CHOICE: Format1 선택 ----
  descriptor->indicationHeader_formats.present =
      E2SM_KPM_IndicationHeader__indicationHeader_formats_PR_indicationHeader_Format1;
  descriptor->indicationHeader_formats.choice.indicationHeader_Format1 = ind_header;

  // ---- 5) 실제 인코딩 ----
  Encode(descriptor);
  ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationHeader_Format1, ind_header);

}

KpmIndicationMessage::KpmIndicationMessage (KpmIndicationMessageValues values, const E2SM_KPM_IndicationMessage_FormatType &format_type)
{
  E2SM_KPM_IndicationMessage_t* descriptor = (E2SM_KPM_IndicationMessage_t*)calloc(1, sizeof(*descriptor));
  CheckConstraints(values);
  FillAndEncodeKpmIndicationMessage(descriptor, values, format_type); 
  //ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationMessage, descriptor);
  //free(descriptor);
}

KpmIndicationMessage::~KpmIndicationMessage ()
{
  free (m_buffer);
  m_size = 0;
}

void
KpmIndicationMessage::CheckConstraints (KpmIndicationMessageValues values)
{
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
    // ---------------------------------------------------------
    // 1) Obtain name / value from input
    // ---------------------------------------------------------
    MeasurementRecordItem_t *recordItem = mesItem->GetRecordItem();
    MeasurementInfoItem_t   *infoItem   = mesItem->GetInfoItem();

    if (!recordItem || !infoItem)
    {
        NS_LOG_WARN("MeasurementItemV2 returned null record or info");
        return {nullptr, nullptr};
    }

     // ---------------------------------------------------------
    // 2) Create MeasurementDataItem_t (KPMv2)
    // ---------------------------------------------------------
    MeasurementDataItem_t *dataItem =
        (MeasurementDataItem_t *)calloc(1, sizeof(MeasurementDataItem_t));

    if (!dataItem)
    {
        NS_LOG_ERROR("calloc failed for MeasurementDataItem");
        return {nullptr, nullptr};
    }

    ASN_SEQUENCE_ADD(&dataItem->measRecord.list, recordItem);


    // ---------------------------------------------------------
    // KPM v2에서는 MeasurementLabel → LabelInfoItem_t 로 구성됨
    // 이 예제에서는 noLabel = true 로 설정
    // ---------------------------------------------------------
    LabelInfoItem_t *labelItem =
        (LabelInfoItem_t *)calloc(1, sizeof(LabelInfoItem_t));

    MeasurementLabel_t *label =
        (MeasurementLabel_t *)calloc(1, sizeof(MeasurementLabel_t));

    label->noLabel = (long *)calloc(1, sizeof(long));
    *label->noLabel = 0;

    labelItem->measLabel = *label;

    ASN_SEQUENCE_ADD(&infoItem->labelInfoList.list, labelItem);

    // ---------------------------------------------------------
    // 최종 반환: (MeasurementInfoItem_t*, MeasurementDataItem_t*)
    // ---------------------------------------------------------
    return {infoItem, dataItem};
  }



MeasurementDataItem_t *
KpmIndicationMessage::getMesDataItem (const double &realVal)
{
  // 1. MeasurementDataItem_t 자체를 먼저 생성
  MeasurementDataItem_t *measure_data_item =
      (MeasurementDataItem_t *)calloc(1, sizeof(MeasurementDataItem_t));
  if (!measure_data_item)
    {
      NS_LOG_ERROR("calloc failed for MeasurementDataItem_t");
      return nullptr;
    }

  // 2. MeasurementRecordItem_t 생성
  MeasurementRecordItem_t *measure_record_item =
      (MeasurementRecordItem_t *)calloc(1, sizeof(MeasurementRecordItem_t));
  if (!measure_record_item)
    {
      NS_LOG_ERROR("calloc failed for MeasurementRecordItem_t");
      ASN_STRUCT_FREE(asn_DEF_MeasurementDataItem, measure_data_item);
      return nullptr;
    }

  measure_record_item->present = MeasurementRecordItem_PR_real;
  measure_record_item->choice.real = realVal;

  // 3. MeasurementDataItem_t 안의 measRecord.list 에 직접 추가
  if (ASN_SEQUENCE_ADD(&measure_data_item->measRecord.list, measure_record_item) != 0)
    {
      NS_LOG_ERROR("ASN_SEQUENCE_ADD failed in getMesDataItem");
      ASN_STRUCT_FREE(asn_DEF_MeasurementRecordItem, measure_record_item);
      ASN_STRUCT_FREE(asn_DEF_MeasurementDataItem, measure_data_item);
      return nullptr;
    }

  return measure_data_item;
}

MeasurementDataItem_t *
KpmIndicationMessage::getMesDataItem (long intVal)
{
  MeasurementDataItem_t *measure_data_item =
      (MeasurementDataItem_t *)calloc(1, sizeof(MeasurementDataItem_t));
  if (!measure_data_item)
    {
      NS_LOG_ERROR("calloc failed for MeasurementDataItem_t");
      return nullptr;
    }

  MeasurementRecordItem_t *measure_record_item =
      (MeasurementRecordItem_t *)calloc(1, sizeof(MeasurementRecordItem_t));
  if (!measure_record_item)
    {
      NS_LOG_ERROR("calloc failed for MeasurementRecordItem_t");
      ASN_STRUCT_FREE(asn_DEF_MeasurementDataItem, measure_data_item);
      return nullptr;
    }

  measure_record_item->present = MeasurementRecordItem_PR_integer;
  measure_record_item->choice.integer = intVal;

  if (ASN_SEQUENCE_ADD(&measure_data_item->measRecord.list, measure_record_item) != 0)
    {
      NS_LOG_ERROR("ASN_SEQUENCE_ADD failed in getMesDataItem(int)");
      ASN_STRUCT_FREE(asn_DEF_MeasurementRecordItem, measure_record_item);
      ASN_STRUCT_FREE(asn_DEF_MeasurementDataItem, measure_data_item);
      return nullptr;
    }

  return measure_data_item;
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

  // ---------------------------------------------------------
  // 1) MeasurementInfoList 생성
  // ---------------------------------------------------------
  MeasurementInfoList_t *infoList =
      (MeasurementInfoList_t *)calloc(1, sizeof(MeasurementInfoList_t));

  auto items = indication->GetItems();

  for (const auto &item : items)
    {
      MeasurementInfoItem_t *info = item->GetInfoItem();
      if (!info)
        {
          NS_LOG_WARN("Skipping MeasurementInfoItem: null");
          continue;
        }

      // labelInfoList는 항상 Sequence이므로 직접 add
      ASN_SEQUENCE_ADD(&infoList->list, info);
    }

  if (infoList->list.count == 0)
    {
      NS_LOG_WARN("No MeasurementInfoItem in KPM Format1");
      ASN_STRUCT_FREE(asn_DEF_MeasurementInfoList, infoList);
      return;
    }

  format->measInfoList = infoList;

  // ---------------------------------------------------------
  // 2) MeasurementDataItem 생성 (셀마다 하나)
  // ---------------------------------------------------------
  MeasurementDataItem_t *dataItem =
      (MeasurementDataItem_t *)calloc(1, sizeof(MeasurementDataItem_t));

  // 이 셀의 모든 KPI 값(MeasurementRecordItem)을 measRecord.list에 추가
  for (const auto &item : items)
    {
      MeasurementRecordItem_t *rec = item->GetRecordItem();
      NS_LOG_DEBUG("Record present" << rec->present);

      if (!rec)
        {
          NS_LOG_WARN("Skipping MeasurementRecordItem: null");
          continue;
        }

      ASN_SEQUENCE_ADD(&dataItem->measRecord.list, rec);
    }

  // ---------------------------------------------------------
  // dataItem을 measurementData에 추가
  // ---------------------------------------------------------
  ASN_SEQUENCE_ADD(&format->measData.list, dataItem);

  // ---------------------------------------------------------
  // 3) GranularityPeriod 설정
  // ---------------------------------------------------------
  GranularityPeriod_t *gran =
      (GranularityPeriod_t *)calloc(1, sizeof(GranularityPeriod_t));
  *gran = 100;

  format->granulPeriod = gran;

  NS_LOG_DEBUG("KPMv2 Format1 created: "
               << "info=" << format->measInfoList->list.count
               << " record=" << dataItem->measRecord.list.count);
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
            auto info = item->GetInfoItem();
            auto rec  = item->GetRecordItem();

            if (!info || !rec)
                continue;

            std::string measName(
                (char *)info->measType.choice.measName.buf,
                info->measType.choice.measName.size);

            rep.metricNames.push_back(measName);
            switch (rec->present)
            {
                case MeasurementRecordItem_PR_integer:
                    rep.metricValues.push_back(rec->choice.integer);
                    break;

                case MeasurementRecordItem_PR_real:
                    rep.metricValues.push_back(rec->choice.real);
                    break;

                default:
                    rep.metricValues.push_back(0.0);
            }
        }

        reports.push_back(rep);
    }

    return reports;
}

void
KpmIndicationMessage::FillKpmIndicationMessageFormat2(
    E2SM_KPM_IndicationMessage_Format2_t *fmt2,
    const KpmIndicationMessageValues &values)
{
  NS_LOG_DEBUG("FillKpmIndicationMessageFormat2(): start, UEs="
               << values.m_ueIndications.size());

  // --------------------------
  // 1) UE report 리스트 추출
  // --------------------------
  auto ueReports = ExtractUeReports(values);

  int ueCount = ueReports.size();
  if (ueCount == 0)
    {
      NS_LOG_WARN("Format2: no UE reports");
      return;   // fmt2는 빈 상태로 남음 -> 호출측에서 measData.count 체크 필요
    }

  int metricCount = ueReports[0].metricNames.size();
  if (metricCount == 0)
    {
      NS_LOG_WARN("Format2: no metrics");
      return;
    }

  // ----------------------------------------------------
  // 2) MeasurementData: 한 개의 MeasurementDataItem
  //    - 여기에서만 새 ASN.1 객체를 만들고, 외부 포인터는 절대 쓰지 않음
  // ----------------------------------------------------
  MeasurementDataItem_t *dataItem =
      (MeasurementDataItem_t *)calloc(1, sizeof(*dataItem));
  if (!dataItem)
    {
      NS_FATAL_ERROR("calloc failed for MeasurementDataItem_t");
    }

  // metric m, UE u 에 대해 하나의 MeasurementRecordItem 생성 (deep copy)
  for (int m = 0; m < metricCount; ++m)
    {
      for (int u = 0; u < ueCount; ++u)
        {
          MeasurementRecordItem_t *rec =
              (MeasurementRecordItem_t *)calloc(1, sizeof(*rec));
          if (!rec)
            {
              NS_FATAL_ERROR("calloc failed for MeasurementRecordItem_t");
            }

          rec->present     = MeasurementRecordItem_PR_real;   // double 값 사용
          rec->choice.real = ueReports[u].metricValues[m];

          if (ASN_SEQUENCE_ADD(&dataItem->measRecord.list, rec) != 0)
            {
              NS_FATAL_ERROR("ASN_SEQUENCE_ADD failed for measRecord in Format2");
            }
        }
    }

  if (dataItem->measRecord.list.count == 0)
    {
      NS_LOG_WARN("Format2: dataItem has 0 MeasurementRecordItem, skip");
      ASN_STRUCT_FREE(asn_DEF_MeasurementDataItem, dataItem);
      return;
    }

  if (ASN_SEQUENCE_ADD(&fmt2->measData.list, dataItem) != 0)
    {
      NS_FATAL_ERROR("ASN_SEQUENCE_ADD failed for fmt2->measData");
    }

  // ----------------------------------------------------
  // 3) MeasurementCondUEidList: metric 단위 조건/label 정의
  //    - 이 부분도 모든 ASN.1 객체를 새로 alloc해서 fmt2에만 귀속
  // ----------------------------------------------------
  for (int m = 0; m < metricCount; ++m)
    {
      MeasurementCondUEidItem_t *item =
          (MeasurementCondUEidItem_t *)calloc(1, sizeof(*item));
      if (!item)
        {
          NS_FATAL_ERROR("calloc failed for MeasurementCondUEidItem_t");
        }

      // 3-1) MeasurementType: measName (metric 이름)
      item->measType.present = MeasurementType_PR_measName;
      const std::string &metricName = ueReports[0].metricNames[m];
      if (OCTET_STRING_fromBuf(&item->measType.choice.measName,
                               metricName.c_str(),
                               metricName.size()) != 0)
        {
          NS_FATAL_ERROR("OCTET_STRING_fromBuf failed for measName in Format2");
        }

      // 3-2) MatchingCondList: SIZE >= 1
      MatchingCondItem_t *mci =
          (MatchingCondItem_t *)calloc(1, sizeof(*mci));
      if (!mci)
        {
          NS_FATAL_ERROR("calloc failed for MatchingCondItem_t");
        }

      mci->present = MatchingCondItem_PR_measLabel;

      // Label도 완전히 새로운 ASN.1 객체로 할당 (no shallow copy)
      MeasurementLabel_t *label =
          (MeasurementLabel_t *)calloc(1, sizeof(*label));
      if (!label)
        {
          NS_FATAL_ERROR("calloc failed for MeasurementLabel_t");
        }

      label->noLabel = (long *)calloc(1, sizeof(long));
      if (!label->noLabel)
        {
          NS_FATAL_ERROR("calloc failed for label->noLabel");
        }
      *label->noLabel = 0; // "noLabel" semantics

      mci->choice.measLabel = label;

      if (ASN_SEQUENCE_ADD(&item->matchingCond.list, mci) != 0)
        {
          NS_FATAL_ERROR("ASN_SEQUENCE_ADD failed for matchingCond.list");
        }

      // matchingUEidList OPTIONAL → 사용 안 함
      item->matchingUEidList = NULL;

      if (ASN_SEQUENCE_ADD(&fmt2->measCondUEidList.list, item) != 0)
        {
          NS_FATAL_ERROR("ASN_SEQUENCE_ADD failed for measCondUEidList");
        }
    }

  // ----------------------------------------------------
  // 4) granularityPeriod 설정
  // ----------------------------------------------------
  GranularityPeriod_t *gran =
      (GranularityPeriod_t *)calloc(1, sizeof(*gran));
  if (!gran)
    {
      NS_FATAL_ERROR("calloc failed for GranularityPeriod_t");
    }
  *gran = 100; // 100ms

  fmt2->granulPeriod = gran;

  NS_LOG_DEBUG("FillKpmIndicationMessageFormat2(): done, measData.count="
               << fmt2->measData.list.count
               << ", measCondUEidList.count="
               << fmt2->measCondUEidList.list.count);
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
KpmIndicationMessage::FillAndEncodeKpmIndicationMessage(
    E2SM_KPM_IndicationMessage_t *descriptor,
    KpmIndicationMessageValues values,
    const E2SM_KPM_IndicationMessage_FormatType &format_type)
{
  // 1) 이전 내용 깨끗이 제거 + 새로 할당
  ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationMessage, descriptor);
  descriptor = (E2SM_KPM_IndicationMessage_t *)calloc(1, sizeof(*descriptor));
  if (!descriptor)
    {
      NS_FATAL_ERROR("calloc failed for E2SM_KPM_IndicationMessage");
    }

  switch (format_type)
    {
    case E2SM_KPM_INDICATION_MESSAGE_FORMART1:
      {
        NS_LOG_DEBUG("Encode E2SM_KPM_I For Cell (Format1)");
        E2SM_KPM_IndicationMessage_Format1_t *msg_fmt1 =
            (E2SM_KPM_IndicationMessage_Format1_t *)calloc(1, sizeof(*msg_fmt1));
        if (!msg_fmt1)
          {
            NS_FATAL_ERROR("calloc failed for msg_fmt1");
          }

        // cell 측정값 확보
        const std::string cellId =
            values.m_cellObjectId.empty() ? "NO_CELL_ID" : values.m_cellObjectId;

        Ptr<MeasurementItemList> cellItems = values.m_cellMeasurementItems;
        if (!cellItems)
          {
            NS_LOG_DEBUG("Creating MeasurementItemList For Cell");
            cellItems = Create<MeasurementItemList>(cellId);
            cellItems->AddItem("DRB.PdcpSduDelayDl", 0.0);
            cellItems->AddItem("pdcpBytesUl",        0.1);
            cellItems->AddItem("pdcpBytesDl",        0.2);
            cellItems->AddItem("numActiveUes",       0.3);
          }

        FillKpmIndicationMessageFormat1(msg_fmt1, cellItems, cellId);

        // measData가 비어있으면 인코딩 안 함
        if (msg_fmt1->measData.list.count == 0)
          {
            NS_LOG_WARN("Format1: measData is empty, skip encoding");
            ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationMessage_Format1, msg_fmt1);
            return;
          }

        descriptor->indicationMessage_formats.present =
            E2SM_KPM_IndicationMessage__indicationMessage_formats_PR_indicationMessage_Format1;
        descriptor->indicationMessage_formats.choice.indicationMessage_Format1 = msg_fmt1;
        break;
      }

    case E2SM_KPM_INDICATION_MESSAGE_FORMART2:
      {
        NS_LOG_DEBUG("Encode E2SM_KPM_I For UE (Format2)");

        E2SM_KPM_IndicationMessage_Format2_t  *fmt2 =
            (E2SM_KPM_IndicationMessage_Format2_t *)calloc(1, sizeof(*fmt2));
        if (!fmt2)
          {
            NS_FATAL_ERROR("calloc failed for E2SM_KPM_IndicationMessage_Format2_t");
          }

        if (!values.m_ueIndications.empty())
          {
            FillKpmIndicationMessageFormat2(fmt2, values);
          }

        // measData가 비면 인코딩하지 않고 정리
        if (fmt2->measData.list.count == 0)
          {
            NS_LOG_WARN("Format2: measData is empty, skip encoding");
            ASN_STRUCT_FREE(asn_DEF_E2SM_KPM_IndicationMessage_Format2, fmt2);
            return;
          }

        descriptor->indicationMessage_formats.present =
            E2SM_KPM_IndicationMessage__indicationMessage_formats_PR_indicationMessage_Format2;
        descriptor->indicationMessage_formats.choice.indicationMessage_Format2 = fmt2;
        break;
      }

    default:
      NS_LOG_WARN("Unknown KPM IndicationMessage format_type");
      return;
    }

  // 3) 실제 인코딩
  Encode(descriptor);
  printf("\n *** Done Encoding INDICATION Message ****** \n");
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

} // namespace ns3
