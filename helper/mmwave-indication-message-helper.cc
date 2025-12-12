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
 *       Mina Yonan <mina.awadallah@orange.com>
 *       Mostafa Ashraf <mostafa.ashraf.ext@orange.com>
 */

#include <ns3/mmwave-indication-message-helper.h>

namespace ns3 {

MmWaveIndicationMessageHelper::MmWaveIndicationMessageHelper (IndicationMessageType type,
                                                              bool isOffline, bool reducedPmValues)
    : IndicationMessageHelper (type, isOffline, reducedPmValues)
{
}

void
MmWaveIndicationMessageHelper::AddgNBUeItem (std::string ueImsiComplete, long numDrb,
                                                long drbRelAct,
                                                long txPdcpPduBytesNrRlc, long txPdcpPduNrRlc, 
                                                long macPduUe, long macPduInitialUe, long macQpsk, 
                                                long mac16Qam, long mac64Qam, long macRetx, 
                                                long macVolume, long macPrb, long macMac04, 
                                                long macMac59, long macMac1014, long macMac1519, 
                                                long macMac2024, long macMac2529, long macSinrBin1,
                                                long macSinrBin2, long macSinrBin3, long macSinrBin4, 
                                                long macSinrBin5, long macSinrBin6, long macSinrBin7, 
                                                long rlcBufferOccup, double drbThrDlUeid,
                                                double sinrServCell, double convertedSinrServCell, uint16_t IDServCell,
                                                double sinrNeigCell1, double convertedSinrNeigCell1,  uint16_t IDNeigCell1,
                                                double sinrNeigCell2, double convertedSinrNeigCell2,  uint16_t IDNeigCell2,
                                                double sinrNeigCell3, double convertedSinrNeigCell3,  uint16_t IDNeigCell3,
                                                double sinrNeigCell4, double convertedSinrNeigCell4,  uint16_t IDNeigCell4,
                                                double sinrNeigCell5, double convertedSinrNeigCell5,  uint16_t IDNeigCell5,
                                                double sinrNeigCell6, double convertedSinrNeigCell6,  uint16_t IDNeigCell6,   
                                                double sinrNeigCell7, double convertedSinrNeigCell7,  uint16_t IDNeigCell7,   
                                                double sinrNeigCell8, double convertedSinrNeigCell8,  uint16_t IDNeigCell8   
                                              )
{

  Ptr<MeasurementItemList> ueVal = Create<MeasurementItemList> (ueImsiComplete);
    long ueImsiLong = std::stoll(ueImsiComplete);

    ueVal->AddItem<long> ("UEID", ueImsiLong);
  if (!m_reducedPmValues)
    {
      //CUCP
      ueVal->AddItem<long> ("DRB.EstabSucc.5QI.UEID", numDrb);
      ueVal->AddItem<long> ("DRB.RelActNbr.5QI.UEID", drbRelAct); // not modeled in the simulator
      // CUUP
      ueVal->AddItem<long> ("QosFlow.PdcpPduVolumeDL_Filter.UEID", txPdcpPduBytesNrRlc);
      ueVal->AddItem<long> ("DRB.PdcpPduNbrDl.Qos.UEID", txPdcpPduNrRlc);
      //DU
      ueVal->AddItem<long> ("TB.TotNbrDl.1.UEID", macPduUe);
      ueVal->AddItem<long> ("TB.TotNbrDlInitial.UEID", macPduInitialUe);
      ueVal->AddItem<long> ("TB.TotNbrDlInitial.Qpsk.UEID", macQpsk);
      ueVal->AddItem<long> ("TB.TotNbrDlInitial.16Qam.UEID", mac16Qam);
      ueVal->AddItem<long> ("TB.TotNbrDlInitial.64Qam.UEID", mac64Qam);
      ueVal->AddItem<long> ("TB.ErrTotalNbrDl.1.UEID", macRetx);
      ueVal->AddItem<long> ("QosFlow.PdcpPduVolumeDL_Filter.UEID", macVolume);
      ueVal->AddItem<long> ("RRU.PrbUsedDl.UEID", (long) std::ceil (macPrb));
      ueVal->AddItem<long> ("CARR.PDSCHMCSDist.Bin1.UEID", macMac04);
      ueVal->AddItem<long> ("CARR.PDSCHMCSDist.Bin2.UEID", macMac59);
      ueVal->AddItem<long> ("CARR.PDSCHMCSDist.Bin3.UEID", macMac1014);
      ueVal->AddItem<long> ("CARR.PDSCHMCSDist.Bin4.UEID", macMac1519);
      ueVal->AddItem<long> ("CARR.PDSCHMCSDist.Bin5.UEID", macMac2024);
      ueVal->AddItem<long> ("CARR.PDSCHMCSDist.Bin6.UEID", macMac2529);
      ueVal->AddItem<long> ("L1M.RS-SINR.Bin34.UEID", macSinrBin1);
      ueVal->AddItem<long> ("L1M.RS-SINR.Bin46.UEID", macSinrBin2);
      ueVal->AddItem<long> ("L1M.RS-SINR.Bin58.UEID", macSinrBin3);
      ueVal->AddItem<long> ("L1M.RS-SINR.Bin70.UEID", macSinrBin4);
      ueVal->AddItem<long> ("L1M.RS-SINR.Bin82.UEID", macSinrBin5);
      ueVal->AddItem<long> ("L1M.RS-SINR.Bin94.UEID", macSinrBin6);
      ueVal->AddItem<long> ("L1M.RS-SINR.Bin127.UEID", macSinrBin7);
      ueVal->AddItem<long> ("DRB.BufferSize.Qos.UEID", rlcBufferOccup);


    }

  // 입력으로 받아야 되는 값 ServCellSINR (sinrThisCell), ServCellID (m_cellId), ServCellSINRConverted (convertedSinr)
  // m_l3sinrMap 에 저장 <sinr (double), cellID (STRING)>
  //


      ueVal->AddItem<double> ("HO.SrcCellQual.RS-SINR.UEID", sinrServCell);
      ueVal->AddItem<double> ("HO.SrcCellQual.RS-SINR-Converted.UEID", convertedSinrServCell);
      ueVal->AddItem<long> ("HO.SrcCellID.UEID", IDServCell);

      ueVal->AddItem<double> ("HO.TrgtCellQual.1.RS-SINR.UEID", sinrNeigCell1);
      ueVal->AddItem<double> ("HO.TrgtCellQual.1.RS-SINR-Converted.UEID", convertedSinrNeigCell1);
      ueVal->AddItem<long> ("HO.TrgtCellQual.1.UEID", IDNeigCell1);

      ueVal->AddItem<double> ("HO.TrgtCellQual.2.RS-SINR.UEID", sinrNeigCell2);
      ueVal->AddItem<double> ("HO.TrgtCellQual.2.RS-SINR-Converted.UEID", convertedSinrNeigCell2);
      ueVal->AddItem<long> ("HO.TrgtCellQual.2.UEID", IDNeigCell2);

      ueVal->AddItem<double> ("HO.TrgtCellQual.3.RS-SINR.UEID", sinrNeigCell3);
      ueVal->AddItem<double> ("HO.TrgtCellQual.3.RS-SINR-Converted.UEID", convertedSinrNeigCell3);
      ueVal->AddItem<long> ("HO.TrgtCellQual.3.UEID", IDNeigCell3);

      ueVal->AddItem<double> ("HO.TrgtCellQual.4.RS-SINR.UEID", sinrNeigCell4);
      ueVal->AddItem<double> ("HO.TrgtCellQual.4.RS-SINR-Converted.UEID", convertedSinrNeigCell4);
      ueVal->AddItem<long> ("HO.TrgtCellQual.4.UEID", IDNeigCell4);

      ueVal->AddItem<double> ("HO.TrgtCellQual.5.RS-SINR.UEID", sinrNeigCell5);
      ueVal->AddItem<double> ("HO.TrgtCellQual.5.RS-SINR-Converted.UEID", convertedSinrNeigCell5);
      ueVal->AddItem<long> ("HO.TrgtCellQual.5.UEID", IDNeigCell5);

      ueVal->AddItem<double> ("HO.TrgtCellQual.6.RS-SINR.UEID", sinrNeigCell6);
      ueVal->AddItem<double> ("HO.TrgtCellQual.6.RS-SINR-Converted.UEID", convertedSinrNeigCell6);
      ueVal->AddItem<long> ("HO.TrgtCellQual.6.UEID", IDNeigCell6);

      ueVal->AddItem<double> ("HO.TrgtCellQual.7.RS-SINR.UEID", sinrNeigCell7);
      ueVal->AddItem<double> ("HO.TrgtCellQual.7.RS-SINR-Converted.UEID", convertedSinrNeigCell7);
      ueVal->AddItem<long> ("HO.TrgtCellQual.7.UEID", IDNeigCell7);

      ueVal->AddItem<double> ("HO.TrgtCellQual.8.RS-SINR.UEID", sinrNeigCell8);
      ueVal->AddItem<double> ("HO.TrgtCellQual.8.RS-SINR-Converted.UEID", convertedSinrNeigCell8);
      ueVal->AddItem<long> ("HO.TrgtCellQual.8.UEID", IDNeigCell8);

  //du
  ueVal->AddItem<double> ("DRB.UEThpDl.UEID", drbThrDlUeid);

  m_msgValues.m_ueIndications.insert (ueVal);
}

void
MmWaveIndicationMessageHelper::AddgNBCellItem (long cellid, uint16_t numActiveUes,
    long macPduCellSpecific, long macPduInitialCellSpecific, long macQpskCellSpecific,
    long mac16QamCellSpecific, long mac64QamCellSpecific, double prbUtilizationDl,
    long macRetxCellSpecific, long macVolumeCellSpecific, long macMac04CellSpecific,
    long macMac59CellSpecific, long macMac1014CellSpecific, long macMac1519CellSpecific,
    long macMac2024CellSpecific, long macMac2529CellSpecific, long macSinrBin1CellSpecific,
    long macSinrBin2CellSpecific, long macSinrBin3CellSpecific, long macSinrBin4CellSpecific,
    long macSinrBin5CellSpecific, long macSinrBin6CellSpecific, long macSinrBin7CellSpecific,
    long rlcBufferOccupCellSpecific, long activeUeDl,
    long dlAvailablePrbs, long ulAvailablePrbs, long qci,
    long dlPrbUsage, long ulPrbUsage)
{
  Ptr<MeasurementItemList> cellVal = Create<MeasurementItemList> ();
      cellVal->AddItem<long> ("cellID", cellid);

    if (!m_reducedPmValues)
    {
      cellVal->AddItem<long> ("TB.TotNbrDl.1", macPduCellSpecific);
      cellVal->AddItem<long> ("TB.TotNbrDlInitial", macPduInitialCellSpecific);
    }


  cellVal->AddItem<long> ("numActiveUes", numActiveUes);
  cellVal->AddItem<long> ("TB.TotNbrDlInitial.Qpsk", macQpskCellSpecific);
  cellVal->AddItem<long> ("TB.TotNbrDlInitial.16Qam", mac16QamCellSpecific);
  cellVal->AddItem<long> ("TB.TotNbrDlInitial.64Qam", mac64QamCellSpecific);
  cellVal->AddItem<long> ("RRU.PrbUsedDl", (long) std::ceil (prbUtilizationDl));
  cellVal->AddItem<long> ("TB.ErrTotalNbrDl.1", macRetxCellSpecific);
  cellVal->AddItem<long> ("QosFlow.PdcpPduVolumeDL_Filter", macVolumeCellSpecific);
  cellVal->AddItem<long> ("CARR.PDSCHMCSDist.Bin1", macMac04CellSpecific);
  cellVal->AddItem<long> ("CARR.PDSCHMCSDist.Bin2", macMac59CellSpecific);
  cellVal->AddItem<long> ("CARR.PDSCHMCSDist.Bin3", macMac1014CellSpecific);
  cellVal->AddItem<long> ("CARR.PDSCHMCSDist.Bin4", macMac1519CellSpecific);
  cellVal->AddItem<long> ("CARR.PDSCHMCSDist.Bin5", macMac2024CellSpecific);
  cellVal->AddItem<long> ("CARR.PDSCHMCSDist.Bin6", macMac2529CellSpecific);
  cellVal->AddItem<long> ("L1M.RS-SINR.Bin34", macSinrBin1CellSpecific);
  cellVal->AddItem<long> ("L1M.RS-SINR.Bin46", macSinrBin2CellSpecific);
  cellVal->AddItem<long> ("L1M.RS-SINR.Bin58", macSinrBin3CellSpecific);
  cellVal->AddItem<long> ("L1M.RS-SINR.Bin70", macSinrBin4CellSpecific);
  cellVal->AddItem<long> ("L1M.RS-SINR.Bin82", macSinrBin5CellSpecific);
  cellVal->AddItem<long> ("L1M.RS-SINR.Bin94", macSinrBin6CellSpecific);
  cellVal->AddItem<long> ("L1M.RS-SINR.Bin127", macSinrBin7CellSpecific);
  cellVal->AddItem<long> ("DRB.BufferSize.Qos", rlcBufferOccupCellSpecific);
  cellVal->AddItem<long> ("DRB.MeanActiveUeDl",activeUeDl);

  cellVal->AddItem<long> ("PRB.AvailableDl",dlAvailablePrbs);
  cellVal->AddItem<long> ("RRB.AvailablePUl",ulAvailablePrbs);
  cellVal->AddItem<long> ("DRB.QCI",qci);
  cellVal->AddItem<long> ("RRB.UseageDl",dlPrbUsage);
  cellVal->AddItem<long> ("RRB.UseageUl",ulPrbUsage);

  m_msgValues.m_cellMeasurementItems = cellVal;

}

MmWaveIndicationMessageHelper::~MmWaveIndicationMessageHelper ()
{
}

} // namespace ns3