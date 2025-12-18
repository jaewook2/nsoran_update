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
 *		   Mostafa Ashraf <mostafa.ashraf.ext@orange.com>
 *       Aya Kamal <aya.kamal.ext@orange.com>
 *       Abdelrhman Soliman <abdelrhman.soliman.ext@orange.com>
 */

#ifndef KPM_INDICATION_H
#define KPM_INDICATION_H

#include <thread>
#include "ns3/object.h"
#include <set>

#include <thread>
#include <mutex>

#include <vector>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

extern "C" {
#include "E2SM-KPM-RANfunction-Description.h"
#include "E2SM-KPM-IndicationHeader.h"
#include "E2SM-KPM-IndicationMessage.h"
#include "asn1c-types.h"

//===================================


#include "GlobalE2node-ID.h"
#include "GlobalE2node-gNB-ID.h"
#include "GlobalE2node-eNB-ID.h"
#include "GlobalE2node-ng-eNB-ID.h"
#include "GlobalE2node-en-gNB-ID.h"
#include "RIC-EventTriggerStyle-Item.h"
#include "RIC-ReportStyle-Item.h"
#include "TimeStamp.h"
#include "MeasurementInfoList.h"
#include "MeasurementRecordItem.h"
#include "MeasurementDataItem.h"
#include "MeasurementInfoItem.h"
#include "MeasurementCondUEidList.h"
#include "MeasurementCondUEidItem.h"
#include "MeasurementType.h"
#include "MatchingCondList.h"
#include "MatchingCondItem.h"
#include "TestCondInfo.h"
#include "TestCond-Type.h"
#include "TestCond-Expression.h"
#include "TestCond-Value.h"
#include "MatchingUEidList.h"
#include "MatchingUEidItem.h"
#include "UEID.h"

#include "LabelInfoItem.h"
#include "MeasurementLabel.h"
#include "UEID-GNB.h"

#include "E2SM-KPM-IndicationHeader-Format1.h"
#include "E2SM-KPM-IndicationMessage-Format1.h"
#include "E2SM-KPM-IndicationMessage-Format2.h"

}

namespace ns3 {

struct UeReport
{
  std::string ueId;
  std::vector<std::string> metricNames;
  std::vector<double> metricValues;
};
enum E2SM_KPM_IndicationMessage_FormatType {
  E2SM_KPM_INDICATION_MESSAGE_FORMART1 = 0,
  E2SM_KPM_INDICATION_MESSAGE_FORMART2,
  E2SM_KPM_INDICATION_MESSAGE_FORMART3
};

/// 추가
class MeasurementItem : public SimpleRefCount<MeasurementItem>
{
public:
  MeasurementItem (std::string name, long value);
  MeasurementItem (std::string name, double value);

  ~MeasurementItem ();

  // v2 핵심: measurementInfo + recordItem
  MeasurementRecordItem_t* GetRecordItem ();
  MeasurementInfoItem_t*   GetInfoItem ();

private:
  void CreateRecordItem (long value);
  void CreateRecordItem (double value);
  void CreateInfoItem(std::string name);


  MeasurementRecordItem_t *m_recordItem;      // 값
  MeasurementInfoItem_t   *m_infoItem;        // 이름, 라벨 등
};


class KpmIndicationHeader : public SimpleRefCount<KpmIndicationHeader>
{
public:
  enum GlobalE2nodeType { gNB = 0, eNB = 1, ng_eNB = 2, en_gNB = 3 };

  const static int TIMESTAMP_LIMIT_SIZE = 8;

  struct KpmRicIndicationHeaderValues
  {
 
    std::string m_gnbId; //!< gNB ID bit string
    uint16_t m_nrCellId; //!< NR, bit string
    std::string m_plmId; //!< PLMN identity, octet string, 3 bytes
    uint64_t m_timestamp;
  };

  KpmIndicationHeader (GlobalE2nodeType nodeType, KpmRicIndicationHeaderValues values);
  ~KpmIndicationHeader ();

  uint64_t time_now_us_clck ();
  OCTET_STRING_t get_time_now_us ();
  static uint64_t octet_string_to_int_64 (OCTET_STRING_t asn);
  static OCTET_STRING_t int_64_to_octet_string (uint64_t value);

  void *m_buffer;
  size_t m_size;

private:
  void FillAndEncodeKpmRicIndicationHeader (E2SM_KPM_IndicationHeader_t *descriptor,
                                            KpmRicIndicationHeaderValues values);
  void Encode (E2SM_KPM_IndicationHeader_t *descriptor);

  GlobalE2nodeType m_nodeType;
};

class MeasurementItemList : public SimpleRefCount<MeasurementItemList>
{
private:
  Ptr<OctetString> m_id; // ID, contains the UE IMSI if used to carry UE-specific measurement items
  std::vector<Ptr<MeasurementItem>> m_items; //!< list of Measurement Information Items
public:
  MeasurementItemList ();
  MeasurementItemList (std::string ueId);
  ~MeasurementItemList ();
  template <class T>
  void
  AddItem (std::string name, T value)
  {
    Ptr<MeasurementItem> item = Create<MeasurementItem> (name, value);
    m_items.push_back (item);
  }

  std::vector<Ptr<MeasurementItem>> GetItems ();
  OCTET_STRING_t GetId ();
};


class KpmIndicationMessage : public SimpleRefCount<KpmIndicationMessage>
{
public:
  /**
    * Holds the values to be used to fill the RIC Indication Message 
    */
  struct KpmIndicationMessageValues
  {
    std::string m_cellObjectId; //!< Cell Object ID
    Ptr<MeasurementItemList>
        m_cellMeasurementItems; //!< list of cell-specific Measurement Information Items
    std::set<Ptr<MeasurementItemList>> m_ueIndications; //!< list of Measurement Information Items
  };

  //KpmIndicationMessage (KpmIndicationMessageValues values);
  KpmIndicationMessage (KpmIndicationMessageValues values, const E2SM_KPM_IndicationMessage_FormatType &format_type = E2SM_KPM_INDICATION_MESSAGE_FORMART3);

  ~KpmIndicationMessage ();

  void *m_buffer;
  size_t m_size;
  // ======================================================================================
  BIT_STRING_t
  cp_amf_region_id_to_bit_string (uint8_t src)
  {
    assert (src < 64);

    BIT_STRING_t dst = {
        .buf = (uint8_t *) malloc (1),
        .size = 1,
        .bits_unused = 0,
    };
    assert (dst.buf != NULL);

    memcpy (dst.buf, &src, 1);

    return dst;
  }

  BIT_STRING_t
  cp_amf_set_id_to_bit_string (uint16_t val)
  {
    assert (val < 1024);

    BIT_STRING_t dst = {0};
    dst.buf = (uint8_t *) calloc (2, sizeof (uint8_t));
    dst.size = 2;
    dst.bits_unused = 6; // unused_bit;
    assert (dst.buf != NULL);

    dst.buf[0] = val; // 0x5555;
    dst.buf[1] = (val >> 8) << 6;

    return dst;
  }

  BIT_STRING_t
  cp_amf_ptr_to_bit_string (uint8_t src)
  {
    assert (src < 64);

    uint8_t tmp = src << 2;

    BIT_STRING_t dst = {.buf = (uint8_t *) malloc (1), .size = 1, .bits_unused = 2};
    assert (dst.buf != NULL);
    memcpy (dst.buf, &tmp, 1);

    return dst;
  }

  OCTET_STRING_t
  cp_plmn_identity_to_octant_string (uint16_t mCC, uint16_t mNC, uint8_t mNCdIGITlENGTH)
  {
    OCTET_STRING_t dst = {0};
    dst.buf = (uint8_t *) calloc (3, sizeof (uint8_t));
    dst.buf[0] = ((((mCC) / 10) % 10) << 4) | (mCC / 100);
    dst.buf[1] = ((mNCdIGITlENGTH == 2 ? 15 : (mNC) / 100) << 4) | (mCC % 10);
    dst.buf[2] = (((mNC) % 10) << 4) | (((mNC) / 10) % 10);
    dst.size = 3;
    return dst;
  }

  OCTET_STRING_t
  cp_plmn_identity_to_octant_string (uint8_t src)
  {
    OCTET_STRING_t dst = {0};
    dst.buf = (uint8_t *) calloc (3, sizeof (uint8_t));
    dst.buf[0] = src << 4;
    dst.buf[1] = src << 4;
    dst.buf[2] = src << 4;
    dst.size = 3;
    return dst;
  }

  //==================================================================================
private:
  static void CheckConstraints (KpmIndicationMessageValues values);

  void FillAndEncodeKpmIndicationMessage (E2SM_KPM_IndicationMessage_t *descriptor,
                                          KpmIndicationMessageValues values,
                                          const E2SM_KPM_IndicationMessage_FormatType &format_type);
  void Encode (E2SM_KPM_IndicationMessage_t *descriptor);

  void FillKpmIndicationMessageFormat1 (E2SM_KPM_IndicationMessage_Format1 *ind_msg_f_1,
                                        const Ptr<MeasurementItemList> ueIndication, 
                                        const std::string& cellObjectId = "");


  std::vector<UeReport> ExtractUeReports(const KpmIndicationMessageValues &values);

  void FillKpmIndicationMessageFormat2 (E2SM_KPM_IndicationMessage_Format2 *ind_msg_f_2,
                                       const KpmIndicationMessageValues &values);


  std::pair<MeasurementInfoItem_t *, MeasurementDataItem_t *>
  getMesInfoItem (const Ptr<MeasurementItem> &mesItem);
  MeasurementDataItem_t * getMesDataItem (const double &realVal);
  MeasurementDataItem_t * getMesDataItem (long intVal);

  void FillUeID (UEID_t *ue_ID, Ptr<MeasurementItemList> ueIndication);
};

  // 1029 update by jlee
  class KpmIndicationPair : public Object
    {
    public:
    // ns-3 매크로 (필수)
    static TypeId GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::KpmIndicationPair")
        .SetParent<Object> ();
        return tid;
    }

    Ptr<KpmIndicationMessage> cell;
    Ptr<KpmIndicationMessage> ue;
    };

} // namespace ns3

#endif /* KPM_INDICATION_H */
