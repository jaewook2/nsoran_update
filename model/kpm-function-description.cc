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
// JUST FOR lte_ue

const char* performance_measurements_ue_lte[] = {
    "UEID",
  "DRB.PdcpSduVolumeDl_Filter.UEID",
  "Tot.PdcpSduNbrDl.UEID",
  "DRB.PdcpSduBitRateDl.UEID",
  "DRB.PdcpSduDelayDl.UEID",
  "DRB.EstabSucc.5QI.UEID",
  "DRB.RelActNbr.5QI.UEID"
  };

int NUMBER_MEASUREMENTS_UE_LTE = 7;

const char* performance_measurements_ue_gnb[] = {
  "UEID",
  "DRB.EstabSucc.5QI.UEID",
  "DRB.RelActNbr.5QI.UEID",
  "QosFlow.PdcpPduVolumeDL_Filter.UEID",
  "DRB.PdcpPduNbrDl.Qos.UEID",
  "TB.TotNbrDl.1.UEID",
  "DRB.RelActNbr.5QI.UEID",
  "TB.TotNbrDlInitial.UEID", 
  "TB.TotNbrDlInitial.Qpsk.UEID", 
  "TB.TotNbrDlInitial.16Qam.UEID", 
  "TB.TotNbrDlInitial.64Qam.UEID",
  "TB.ErrTotalNbrDl.1.UEID",
  "QosFlow.PdcpPduVolumeDL_Filter.UEID", 
  "RRU.PrbUsedDl.UEID",
  "CARR.PDSCHMCSDist.Bin1.UEID",
  "CARR.PDSCHMCSDist.Bin2.UEID",
  "CARR.PDSCHMCSDist.Bin3.UEID",
  "CARR.PDSCHMCSDist.Bin4.UEID",
  "CARR.PDSCHMCSDist.Bin5.UEID",
  "CARR.PDSCHMCSDist.Bin6.UEID",
  "L1M.RS-SINR.Bin34.UEID",
  "L1M.RS-SINR.Bin46.UEID",
  "L1M.RS-SINR.Bin58.UEID",
  "L1M.RS-SINR.Bin70.UEID",
  "L1M.RS-SINR.Bin82.UEID",
  "L1M.RS-SINR.Bin94.UEID",
  "L1M.RS-SINR.Bin127.UEID",
  "DRB.BufferSize.Qos.UEID",
  "HO.SrcCellQual.RS-SINR.UEID",
  "HO.SrcCellQual.RS-SINR-Converted.UEID",
  "HO.SrcCellID.UEID",
  "HO.TrgtCellQual.1.RS-SINR.UEID", 
  "HO.TrgtCellQual.1.RS-SINR-Converted.UEID", 
  "HO.TrgtCellQual.1.UEID", 
  "HO.TrgtCellQual.2.RS-SINR.UEID", 
  "HO.TrgtCellQual.2.RS-SINR-Converted.UEID", 
  "HO.TrgtCellQual.2.UEID", 
  "HO.TrgtCellQual.3.RS-SINR.UEID", 
  "HO.TrgtCellQual.3.RS-SINR-Converted.UEID", 
  "HO.TrgtCellQual.3.UEID", 
  "HO.TrgtCellQual.4.RS-SINR.UEID", 
  "HO.TrgtCellQual.4.RS-SINR-Converted.UEID", 
  "HO.TrgtCellQual.4.UEID", 
  "HO.TrgtCellQual.5.RS-SINR.UEID", 
  "HO.TrgtCellQual.5.RS-SINR-Converted.UEID", 
  "HO.TrgtCellQual.5.UEID", 
  "HO.TrgtCellQual.6.RS-SINR.UEID", 
  "HO.TrgtCellQual.6.RS-SINR-Converted.UEID", 
  "HO.TrgtCellQual.6.UEID", 
  "HO.TrgtCellQual.7.RS-SINR.UEID", 
  "HO.TrgtCellQual.7.RS-SINR-Converted.UEID", 
  "HO.TrgtCellQual.7.UEID", 
  "HO.TrgtCellQual.8.RS-SINR.UEID", 
  "HO.TrgtCellQual.8.RS-SINR-Converted.UEID", 
  "HO.TrgtCellQual.8.UEID", 
  "DRB.UEThpDl.UEID"
};

int NUMBER_MEASUREMENTS_UE_GNB = 56; //56

// JUST FOR lte_cell
const char* performance_measurements_cell_lte[] = {
  "cellID",
  "DRB.PdcpSduDelayDl",
  "pdcpBytesUl",
  "pdcpBytesDl",
  "numActiveUes"
};
int NUMBER_MEASUREMENTS_CELL_LTE = 5; // 4

// JUST FOR lte_cell
const char* performance_measurements_cell_gnb[] = {
  "cellID",
  "TB.TotNbrDl.1",
  "TB.TotNbrDlInitial",
  "numActiveUes",
  "TB.TotNbrDlInitial.Qpsk",
  "TB.TotNbrDlInitial.16Qam",
  "TB.TotNbrDlInitial.64Qam",
  "RRU.PrbUsedDl",
  "TB.ErrTotalNbrDl.1",
  "QosFlow.PdcpPduVolumeDL_Filter",
  "CARR.PDSCHMCSDist.Bin1",
  "CARR.PDSCHMCSDist.Bin2",
  "CARR.PDSCHMCSDist.Bin３",
  "CARR.PDSCHMCSDist.Bin４",
  "CARR.PDSCHMCSDist.Bin５",
  "CARR.PDSCHMCSDist.Bin６",
  "L1M.RS-SINR.Bin34",
  "L1M.RS-SINR.Bin46",
  "L1M.RS-SINR.Bin58",
  "L1M.RS-SINR.Bin70",
  "L1M.RS-SINR.Bin82",
  "L1M.RS-SINR.Bin94",
  "L1M.RS-SINR.Bin127",
  "DRB.BufferSize.Qos",
  "DRB.MeanActiveUeDl",
  "PRB.AvailableDl",
  "RRB.AvailablePUl",
  "DRB.QCI",
  "RRB.UseageDl",
  "RRB.UseageUl"
  };
int NUMBER_MEASUREMENTS_CELL_GNB = 30; // 4


KpmFunctionDescription::KpmFunctionDescription (int nb_type)
{
  NS_LOG_DEBUG ("Create KPM Function Descrption");
  E2SM_KPM_RANfunction_Description_t *descriptor = new E2SM_KPM_RANfunction_Description_t ();
  NS_LOG_DEBUG ("Create KPM Function Descrption");

  FillAndEncodeKpmFunctionDescription (descriptor, nb_type);
  NS_LOG_DEBUG ("Create KPM Function Descrption Done");

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
  
  asn_codec_ctx_t *opt_cod = 0; // disable stack bounds checking

  size_t e2smbuffer_size = 8192;
  uint8_t* e2smbuffer = (uint8_t*) malloc(e2smbuffer_size);
  if (!e2smbuffer) {
      NS_FATAL_ERROR("Failed to allocate memory for e2smbuffer");
  }
  NS_LOG_DEBUG("Encoding Start1");

  asn_enc_rval_t er =
    asn_encode_to_buffer(opt_cod,
          ATS_ALIGNED_BASIC_PER,
          &asn_DEF_E2SM_KPM_RANfunction_Description,
          descriptor, e2smbuffer, e2smbuffer_size);
  NS_LOG_DEBUG("Encoding Start2");

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

// update
void
KpmFunctionDescription::FillAndEncodeKpmFunctionDescription (
    E2SM_KPM_RANfunction_Description_t *ranfunc_desc, int nb_type)
{

  uint8_t* short_name = (uint8_t*)"ORAN-E2SM-KPM";
  uint8_t* func_desc = (uint8_t*)"O-RAN E2SM KPM Function";
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

  MeasurementInfo_Action_List_t* measInfo_Action_List_ue_lte =
      (MeasurementInfo_Action_List_t*)calloc(1, sizeof(MeasurementInfo_Action_List_t));

  for (int i = 0; i < NUMBER_MEASUREMENTS_UE_LTE; i++) {
    uint8_t* metrics = (uint8_t *)performance_measurements_ue_lte[i];
    MeasurementInfo_Action_Item_t* measItem =
        (MeasurementInfo_Action_Item_t*)calloc(1, sizeof(MeasurementInfo_Action_Item_t));
    measItem->measName.buf = (uint8_t*)calloc(1, strlen((char*)metrics));
    memcpy(measItem->measName.buf, metrics, strlen((char*)metrics));

    measItem->measName.size = strlen((char*)metrics);

    measItem->measID = (MeasurementTypeID_t*)calloc(1, sizeof(MeasurementTypeID_t));
    *measItem->measID = i+1;

    ASN_SEQUENCE_ADD(&measInfo_Action_List_ue_lte->list, measItem);
  }

    MeasurementInfo_Action_List_t* measInfo_Action_List_ue_gnb =
      (MeasurementInfo_Action_List_t*)calloc(1, sizeof(MeasurementInfo_Action_List_t));

  for (int i = 0; i < NUMBER_MEASUREMENTS_UE_GNB; i++) {
    uint8_t* metrics = (uint8_t *)performance_measurements_ue_gnb[i];
    MeasurementInfo_Action_Item_t* measItem =
        (MeasurementInfo_Action_Item_t*)calloc(1, sizeof(MeasurementInfo_Action_Item_t));
    measItem->measName.buf = (uint8_t*)calloc(1, strlen((char*)metrics));
    memcpy(measItem->measName.buf, metrics, strlen((char*)metrics));

    measItem->measName.size = strlen((char*)metrics);

    measItem->measID = (MeasurementTypeID_t*)calloc(1, sizeof(MeasurementTypeID_t));
    *measItem->measID = i+1;

    ASN_SEQUENCE_ADD(&measInfo_Action_List_ue_gnb->list, measItem);
  }


  MeasurementInfo_Action_List_t* measInfo_Action_List_cell_lte =
      (MeasurementInfo_Action_List_t*)calloc(1, sizeof(MeasurementInfo_Action_List_t));

  for (int i = 0; i < NUMBER_MEASUREMENTS_CELL_LTE; i++) {
    uint8_t* metrics = (uint8_t *)performance_measurements_cell_lte[i];
    MeasurementInfo_Action_Item_t* measItem =
        (MeasurementInfo_Action_Item_t*)calloc(1, sizeof(MeasurementInfo_Action_Item_t));
    measItem->measName.buf = (uint8_t*)calloc(1, strlen((char*)metrics));
    memcpy(measItem->measName.buf, metrics, strlen((char*)metrics));

    measItem->measName.size = strlen((char*)metrics);

    measItem->measID = (MeasurementTypeID_t*)calloc(1, sizeof(MeasurementTypeID_t));
    *measItem->measID = i+1;

    ASN_SEQUENCE_ADD(&measInfo_Action_List_cell_lte->list, measItem);
  }

  MeasurementInfo_Action_List_t* measInfo_Action_List_cell_gnb =
      (MeasurementInfo_Action_List_t*)calloc(1, sizeof(MeasurementInfo_Action_List_t));

  for (int i = 0; i < NUMBER_MEASUREMENTS_CELL_GNB; i++) {
    uint8_t* metrics = (uint8_t *)performance_measurements_cell_gnb[i];
    MeasurementInfo_Action_Item_t* measItem =
        (MeasurementInfo_Action_Item_t*)calloc(1, sizeof(MeasurementInfo_Action_Item_t));
    measItem->measName.buf = (uint8_t*)calloc(1, strlen((char*)metrics));
    memcpy(measItem->measName.buf, metrics, strlen((char*)metrics));

    measItem->measName.size = strlen((char*)metrics);

    measItem->measID = (MeasurementTypeID_t*)calloc(1, sizeof(MeasurementTypeID_t));
    *measItem->measID = i+1;

    ASN_SEQUENCE_ADD(&measInfo_Action_List_cell_gnb->list, measItem);
  }


  ranfunc_desc->ric_ReportStyle_List = (E2SM_KPM_RANfunction_Description::E2SM_KPM_RANfunction_Description__ric_ReportStyle_List*) calloc(1, sizeof(E2SM_KPM_RANfunction_Description::E2SM_KPM_RANfunction_Description__ric_ReportStyle_List));
  
    // UE와 Cell 에따라 변경 + NR / ENB냐에 따라 변경
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

  // report_meastype : 1 cell, 0 UE
  // nb_type : 1 NR, 2 eNB
  auto make_report_style = [&](int type, const char *name, int actionFmt, int hdrFmt, int msgFmt, int report_meastype, int nb_type) {
    RIC_ReportStyle_Item_t *style = (RIC_ReportStyle_Item_t *)calloc(1, sizeof(RIC_ReportStyle_Item_t));
    style->ric_ReportStyle_Type = type;
    OCTET_STRING_fromBuf(&style->ric_ReportStyle_Name, name, -1);
    style->ric_ActionFormat_Type = actionFmt;
    style->ric_IndicationHeaderFormat_Type = hdrFmt;
    style->ric_IndicationMessageFormat_Type = msgFmt;
    if (report_meastype == 0 && nb_type == 0) {  //LTE UE
      style->measInfo_Action_List = *clone_measInfoList(measInfo_Action_List_ue_lte);
    } else if (report_meastype ==  0 && nb_type == 1) { // GNB UE 
      style->measInfo_Action_List = *clone_measInfoList(measInfo_Action_List_ue_gnb);
    } else if (report_meastype ==  1 && nb_type == 0) { //LTE Cell
      style->measInfo_Action_List = *clone_measInfoList(measInfo_Action_List_cell_lte);
    } else if (report_meastype ==  1 && nb_type == 1) { // GNB Cell 
      style->measInfo_Action_List = *clone_measInfoList(measInfo_Action_List_cell_gnb);
    } 
    ASN_SEQUENCE_ADD(&ranfunc_desc->ric_ReportStyle_List->list, style);
  };

  make_report_style(1, "E2 Node Measurement", 1, 1, 1, 1, nb_type);
  make_report_style(2, "E2 Node Measurement for a single UE", 2, 1, 1, 0,nb_type);
  make_report_style(3, "UE-ReportStyle", 3, 1, 2, 0,nb_type);
  make_report_style(4, "Common Condition-based, UE-level Measurement", 4, 1, 3, 1,nb_type);
  make_report_style(5, "E2 Node Measurement for multiple UEs", 5, 1, 3, 0,nb_type);


  Encode (ranfunc_desc);

  NS_LOG_INFO (xer_fprint (stderr, &asn_DEF_E2SM_KPM_RANfunction_Description, ranfunc_desc));
}



} // namespace ns3
