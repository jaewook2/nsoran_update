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


#include <ns3/lte-indication-message-helper.h>

namespace ns3 {

LteIndicationMessageHelper::LteIndicationMessageHelper (IndicationMessageType type, bool isOffline,
                                                        bool reducedPmValues)
    : IndicationMessageHelper (type, isOffline, reducedPmValues)
{
  NS_ABORT_MSG_IF (type == IndicationMessageType::gNB,
                   "Wrong type for LTE Indication Message, expected eNB");
}

// update user plan measurements 
void
LteIndicationMessageHelper::AddeNBUePmItem (std::string ueImsiComplete, long txBytes,
                                             long txDlPackets, double pdcpThroughput,
                                             double pdcpLatency,  long numDrb,
                                             long drbRelAct)
{
  Ptr<MeasurementItemList> ueVal = Create<MeasurementItemList> (ueImsiComplete);
  long ueImsiLong = std::stoll(ueImsiComplete);
    ueVal->AddItem<long> ("UEID", ueImsiLong);
    ueVal->AddItem<long> ("DRB.PdcpSduVolumeDl_Filter.UEID", txBytes);
    ueVal->AddItem<long> ("Tot.PdcpSduNbrDl.UEID", txDlPackets);
    ueVal->AddItem<double> ("DRB.PdcpSduBitRateDl.UEID", pdcpThroughput);
    ueVal->AddItem<double> ("DRB.PdcpSduDelayDl.UEID", pdcpLatency);
    ueVal->AddItem<long> ("DRB.EstabSucc.5QI.UEID", numDrb);
    ueVal->AddItem<long> ("DRB.RelActNbr.5QI.UEID", drbRelAct); // not modeled in the simulator


  m_msgValues.m_ueIndications.insert (ueVal);
}


void
LteIndicationMessageHelper::AddeNBCellPmItem (long cellid, double cellAverageLatency, long pdcpBytesUl, long pdcpBytesDl, uint16_t numActiveUes)
{
    Ptr<MeasurementItemList> cellVal = Create<MeasurementItemList> ();
  
    cellVal->AddItem<long> ("cellID", cellid);
    cellVal->AddItem<double> ("DRB.PdcpSduDelayDl", cellAverageLatency);
    cellVal->AddItem<double> ("pdcpBytesUl", pdcpBytesUl);
    cellVal->AddItem<double> ("pdcpBytesDl", pdcpBytesDl);
    cellVal->AddItem<double> ("numActiveUes", numActiveUes);
    m_msgValues.m_cellMeasurementItems = cellVal;


    
}
LteIndicationMessageHelper::~LteIndicationMessageHelper ()
{
}

} // namespace ns3