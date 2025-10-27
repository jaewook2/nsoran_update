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
 *       Mostafa Ashraf <mostafa.ashraf.ext@orange.com>
 *       Aya Kamal <aya.kamal.ext@orange.com>
 *       Abdelrhman Soliman <abdelrhman.soliman.ext@orange.com>
 */

#include <ns3/kpm-function-description.h>
#include <ns3/asn1c-types.h>
#include <ns3/log.h>

extern "C" {
#include "RIC-EventTriggerStyle-Item.h"
#include "RIC-ReportStyle-Item.h"
}

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("KpmFunctionDescription");
// JUST FOR TEST
const char* performance_measurements[] = {
  "DRB.RlcSduTransmittedVolumeDL",
  "DRB.RlcSduTransmittedVolumeUL",
  "DRB.PerDataVolumeDLDist.Bin",
  "DRB.PerDataVolumeULDist.Bin",
  "DRB.RlcPacketDropRateDLDist",
  "DRB.PacketLossRateULDist",
  "L1M.DL-SS-RSRP.SSB",
  "L1M.DL-SS-SINR.SSB",
  "L1M.UL-SRS-RSRP"
  };

int NUMBER_MEASUREMENTS = 9;

KpmFunctionDescription::KpmFunctionDescription ()
{
  E2SM_KPM_RANfunction_Description_t *descriptor = new E2SM_KPM_RANfunction_Description_t ();
  FillAndEncodeKpmFunctionDescription (descriptor);
  ASN_STRUCT_FREE_CONTENTS_ONLY (asn_DEF_E2SM_KPM_RANfunction_Description, descriptor);
  delete descriptor;
}

KpmFunctionDescription::~KpmFunctionDescription ()
{
  free (m_buffer);
  m_size = 0;
}

void
KpmFunctionDescription::Encode (E2SM_KPM_RANfunction_Description_t *descriptor)
{
  /*
  if (false) {
    asn_codec_ctx_t *opt_cod = 0; // disable stack bounds checking
    // encode the structure into the e2smbuffer
    asn_encode_to_new_buffer_result_s encodedMsg = asn_encode_to_new_buffer (
        opt_cod, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2SM_KPM_RANfunction_Description, descriptor);

    if (encodedMsg.result.encoded < 0)
      {        
        NS_FATAL_ERROR ("Error during the encoding of the RIC Indication Header, errno: "
                        << strerror (errno) << ", failed_type " << encodedMsg.result.failed_type->name
                        << ", structure_ptr " << encodedMsg.result.structure_ptr);
      }

    m_buffer = encodedMsg.buffer;
    m_size = encodedMsg.result.encoded;
  } else {
    asn_codec_ctx_t *opt_cod = 0; // disable stack bounds checking
    // encode the structure into the e2smbuffer
    Bytee_array_t ba = {.buf = (uint8_t *) malloc (2048), .len = 2048};  //2048
    asn_enc_rval_t encodedMsg = asn_encode_to_buffer(
        opt_cod, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2SM_KPM_RANfunction_Description, descriptor,ba.buf,ba.len);

    if (encodedMsg.encoded < 0)
      {        
        NS_FATAL_ERROR ("Error during the encoding of the RIC Indication Header, errno: "
                        << strerror (errno) << ", failed_type " << encodedMsg.failed_type->name
                        << ", structure_ptr " << encodedMsg.structure_ptr);
      }

    m_buffer = ba.buf;
    m_size = encodedMsg.encoded;
  }
    */
  asn_codec_ctx_t *opt_cod = 0; // disable stack bounds checking

  size_t e2smbuffer_size = 8192;
  uint8_t* e2smbuffer = (uint8_t*) malloc(e2smbuffer_size);
  if (!e2smbuffer) {
      NS_FATAL_ERROR("Failed to allocate memory for e2smbuffer");
  }

  asn_enc_rval_t er =
    asn_encode_to_buffer(opt_cod,
          ATS_ALIGNED_BASIC_PER,
          &asn_DEF_E2SM_KPM_RANfunction_Description,
          descriptor, e2smbuffer, e2smbuffer_size);

  if (er.encoded < 0) {
      NS_FATAL_ERROR("Encoding failed: errno: "
        << strerror(errno) << ", failed_type: "
        << er.failed_type->name);
      free(e2smbuffer);  // encoding 실패 시도 메모리 해제
      return;
  }

  m_buffer = e2smbuffer;  // 이 포인터는 이후 free 필요
  m_size = er.encoded;
}


OCTET_STRING_t KpmFunctionDescription::cp_str_to_ba(const char* str)
{
    
    assert(str != NULL);
    const size_t sz = strlen(str);
    OCTET_STRING_t asn = {0};

    asn.buf = (uint8_t*)calloc(sz,sizeof(uint8_t));
      assert(asn.buf != NULL && "Memory exhausted");

      memcpy(asn.buf, str, sz);

    // asn.buf = (uint8_t*) calloc(sizeof(x) + 1, sizeof(char));
    // memcpy(asn.buf,&x,sizeof(x));
    // asn.size = sizeof(x);

    return asn;
}

void
KpmFunctionDescription::FillAndEncodeKpmFunctionDescription (
    E2SM_KPM_RANfunction_Description_t *ranfunc_desc)
{

  uint8_t* short_name = (uint8_t*)"ORAN-E2SM-KPM";
  uint8_t* func_desc = (uint8_t*)"KPM Monitor";
  uint8_t* e2sm_odi = (uint8_t*)"1.3.6.1.4.1.53148.1.2.2.1";
  
  ASN_STRUCT_RESET(asn_DEF_E2SM_KPM_RANfunction_Description, ranfunc_desc);

  ranfunc_desc->ranFunction_Name.ranFunction_ShortName.size = strlen((char*)short_name);
  ranfunc_desc->ranFunction_Name.ranFunction_ShortName.buf =  (uint8_t*)calloc(strlen((char*)short_name), sizeof(uint8_t));
  memcpy(ranfunc_desc->ranFunction_Name.ranFunction_ShortName.buf, short_name,
         ranfunc_desc->ranFunction_Name.ranFunction_ShortName.size);

  ranfunc_desc->ranFunction_Name.ranFunction_Description.buf = (uint8_t*)calloc(1, strlen((char*)func_desc));
  memcpy(ranfunc_desc->ranFunction_Name.ranFunction_Description.buf, func_desc, strlen((char*)func_desc));
  ranfunc_desc->ranFunction_Name.ranFunction_Description.size = strlen((char*)func_desc);
  ranfunc_desc->ranFunction_Name.ranFunction_Instance = (long *)calloc(1, sizeof(long));
  *ranfunc_desc->ranFunction_Name.ranFunction_Instance = 1;

  ranfunc_desc->ranFunction_Name.ranFunction_E2SM_OID.buf = (uint8_t*)calloc(1, strlen((char*)e2sm_odi));
  memcpy(ranfunc_desc->ranFunction_Name.ranFunction_E2SM_OID.buf, e2sm_odi, strlen((char*)e2sm_odi));
  ranfunc_desc->ranFunction_Name.ranFunction_E2SM_OID.size = strlen((char*)e2sm_odi);

  RIC_EventTriggerStyle_Item_t* trigger_style = (RIC_EventTriggerStyle_Item_t*)calloc(1, sizeof(RIC_EventTriggerStyle_Item_t));
  trigger_style->ric_EventTriggerStyle_Type = 1;
  uint8_t* style_name = (uint8_t*)"Periodic Report";
  trigger_style->ric_EventTriggerStyle_Name.buf = (uint8_t*)calloc(1, strlen((char*)style_name));
  memcpy(trigger_style->ric_EventTriggerStyle_Name.buf, style_name, strlen((char*)style_name));
  trigger_style->ric_EventTriggerStyle_Name.size = strlen((char*)style_name);
  trigger_style->ric_EventTriggerFormat_Type = 1; // PREVIOUS VERSION, THIS IS SET TO 0

  ranfunc_desc->ric_EventTriggerStyle_List =
    (E2SM_KPM_RANfunction_Description::E2SM_KPM_RANfunction_Description__ric_EventTriggerStyle_List*)
        calloc(1, sizeof(E2SM_KPM_RANfunction_Description::E2SM_KPM_RANfunction_Description__ric_EventTriggerStyle_List));

  ASN_SEQUENCE_ADD(&ranfunc_desc->ric_EventTriggerStyle_List->list, trigger_style);

  MeasurementInfo_Action_List_t* measInfo_Action_List =
      (MeasurementInfo_Action_List_t*)calloc(1, sizeof(MeasurementInfo_Action_List_t));

  for (int i = 0; i < NUMBER_MEASUREMENTS; i++) {
    uint8_t* metrics = (uint8_t *)performance_measurements[i];
    MeasurementInfo_Action_Item_t* measItem =
        (MeasurementInfo_Action_Item_t*)calloc(1, sizeof(MeasurementInfo_Action_Item_t));
    measItem->measName.buf = (uint8_t*)calloc(1, strlen((char*)metrics));
    memcpy(measItem->measName.buf, metrics, strlen((char*)metrics));

    measItem->measName.size = strlen((char*)metrics);

    measItem->measID = (MeasurementTypeID_t*)calloc(1, sizeof(MeasurementTypeID_t));
    *measItem->measID = i+1;

    ASN_SEQUENCE_ADD(&measInfo_Action_List->list, measItem);
  }
  /*
  RIC_ReportStyle_Item_t* report_style1 = (RIC_ReportStyle_Item_t*)calloc(1, sizeof(RIC_ReportStyle_Item_t));
  report_style1->ric_ReportStyle_Type = 1;
  uint8_t* buf5 = (uint8_t*)"E2 Node Measurement";
  report_style1->ric_ReportStyle_Name.buf = (uint8_t*)calloc(1, strlen((char*)buf5));
  memcpy(report_style1->ric_ReportStyle_Name.buf, buf5, strlen((char*)buf5));
  report_style1->ric_ReportStyle_Name.size = strlen((char*)buf5);
  report_style1->ric_ActionFormat_Type = 1;
  report_style1->ric_IndicationHeaderFormat_Type = 1;
  report_style1->ric_IndicationMessageFormat_Type = 1;
  report_style1->measInfo_Action_List = *measInfo_Action_List;

  RIC_ReportStyle_Item_t* report_style2 = (RIC_ReportStyle_Item_t*)calloc(1, sizeof(RIC_ReportStyle_Item_t));
  report_style2->ric_ReportStyle_Type = 2;
  uint8_t* buf6 = (uint8_t*)"E2 Node Measurement for a single UE";
  report_style2->ric_ReportStyle_Name.buf = (uint8_t*)calloc(1, strlen((char*)buf6));
  memcpy(report_style2->ric_ReportStyle_Name.buf, buf6, strlen((char*)buf6));
  report_style2->ric_ReportStyle_Name.size = strlen((char*)buf6);
  report_style2->ric_ActionFormat_Type = 2;
  report_style2->ric_IndicationHeaderFormat_Type = 1;
  report_style2->ric_IndicationMessageFormat_Type = 1;
  report_style2->measInfo_Action_List = *measInfo_Action_List;

  RIC_ReportStyle_Item_t* report_style3 = (RIC_ReportStyle_Item_t*)calloc(1, sizeof(RIC_ReportStyle_Item_t));
  report_style3->ric_ReportStyle_Type = 3;
  uint8_t* buf7 = (uint8_t*)"Condition-based, UE-level E2 Node Measurement";
  report_style3->ric_ReportStyle_Name.buf = (uint8_t*)calloc(1, strlen((char*)buf7));
  memcpy(report_style3->ric_ReportStyle_Name.buf, buf7, strlen((char*)buf7));
  report_style3->ric_ReportStyle_Name.size = strlen((char*)buf7);
  report_style3->ric_ActionFormat_Type = 3;
  report_style3->ric_IndicationHeaderFormat_Type = 1;
  report_style3->ric_IndicationMessageFormat_Type = 2;
  report_style3->measInfo_Action_List = *measInfo_Action_List;

  RIC_ReportStyle_Item_t* report_style4 =(RIC_ReportStyle_Item_t*)calloc(1, sizeof(RIC_ReportStyle_Item_t));
  report_style4->ric_ReportStyle_Type = 4;
  uint8_t* buf8 = (uint8_t*)"Common Condition-based, UE-level Measurement";
  report_style4->ric_ReportStyle_Name.buf = (uint8_t*)calloc(1, strlen((char*)buf8));
  memcpy(report_style4->ric_ReportStyle_Name.buf, buf8, strlen((char*)buf8));
  report_style4->ric_ReportStyle_Name.size = strlen((char*)buf8);
  report_style4->ric_ActionFormat_Type = 4;
  report_style4->ric_IndicationHeaderFormat_Type = 1;
  report_style4->ric_IndicationMessageFormat_Type = 3;
  report_style4->measInfo_Action_List = *measInfo_Action_List;

  RIC_ReportStyle_Item_t* report_style5 = (RIC_ReportStyle_Item_t*)calloc(1, sizeof(RIC_ReportStyle_Item_t));
  report_style5->ric_ReportStyle_Type = 5;
  uint8_t* buf9 = (uint8_t*)"E2 Node Measurement for multiple UEs";
  report_style5->ric_ReportStyle_Name.buf = (uint8_t*)calloc(1, strlen((char*)buf9));
  memcpy(report_style5->ric_ReportStyle_Name.buf, buf9, strlen((char*)buf9));
  report_style5->ric_ReportStyle_Name.size = strlen((char*)buf9);
  report_style5->ric_ActionFormat_Type = 5;
  report_style5->ric_IndicationHeaderFormat_Type = 1;
  report_style5->ric_IndicationMessageFormat_Type = 3;
  report_style5->measInfo_Action_List = *measInfo_Action_List; 
   */
  ranfunc_desc->ric_ReportStyle_List = (E2SM_KPM_RANfunction_Description::E2SM_KPM_RANfunction_Description__ric_ReportStyle_List*) calloc(1, sizeof(E2SM_KPM_RANfunction_Description::E2SM_KPM_RANfunction_Description__ric_ReportStyle_List));
  /*
  ASN_SEQUENCE_ADD(&ranfunc_desc->ric_ReportStyle_List->list, report_style1);
  ASN_SEQUENCE_ADD(&ranfunc_desc->ric_ReportStyle_List->list, report_style2);
  ASN_SEQUENCE_ADD(&ranfunc_desc->ric_ReportStyle_List->list, report_style3);
  ASN_SEQUENCE_ADD(&ranfunc_desc->ric_ReportStyle_List->list, report_style4);
  ASN_SEQUENCE_ADD(&ranfunc_desc->ric_ReportStyle_List->list, report_style5);
  */

    // ----- Helper: Deep Copy Function -----
  auto clone_measInfoList = [&](MeasurementInfo_Action_List_t *src) {
    MeasurementInfo_Action_List_t *dst = (MeasurementInfo_Action_List_t *)calloc(1, sizeof(*dst));
    for (int i = 0; i < src->list.count; i++) {
      MeasurementInfo_Action_Item_t *srcItem = src->list.array[i];
      MeasurementInfo_Action_Item_t *dstItem =
          (MeasurementInfo_Action_Item_t *)calloc(1, sizeof(MeasurementInfo_Action_Item_t));

      OCTET_STRING_fromBuf(&dstItem->measName, (const char *)srcItem->measName.buf, srcItem->measName.size);
      dstItem->measID = (MeasurementTypeID_t *)calloc(1, sizeof(MeasurementTypeID_t));
      *dstItem->measID = *srcItem->measID;
      ASN_SEQUENCE_ADD(&dst->list, dstItem);
    }
    return dst;
  };
  
  auto make_report_style = [&](int type, const char *name, int actionFmt, int hdrFmt, int msgFmt) {
    RIC_ReportStyle_Item_t *style = (RIC_ReportStyle_Item_t *)calloc(1, sizeof(RIC_ReportStyle_Item_t));
    style->ric_ReportStyle_Type = type;
    OCTET_STRING_fromBuf(&style->ric_ReportStyle_Name, name, -1);
    style->ric_ActionFormat_Type = actionFmt;
    style->ric_IndicationHeaderFormat_Type = hdrFmt;
    style->ric_IndicationMessageFormat_Type = msgFmt;
    style->measInfo_Action_List = *clone_measInfoList(measInfo_Action_List);
    ASN_SEQUENCE_ADD(&ranfunc_desc->ric_ReportStyle_List->list, style);
  };

  
  make_report_style(1, "E2 Node Measurement", 1, 1, 1);
  make_report_style(2, "E2 Node Measurement for a single UE", 2, 1, 1);
  make_report_style(3, "Condition-based, UE-level E2 Node Measurement", 3, 1, 2);
  make_report_style(4, "Common Condition-based, UE-level Measurement", 4, 1, 3);
  make_report_style(5, "E2 Node Measurement for multiple UEs", 5, 1, 3);


  Encode (ranfunc_desc);

  NS_LOG_INFO (xer_fprint (stderr, &asn_DEF_E2SM_KPM_RANfunction_Description, ranfunc_desc));
}



} // namespace ns3
