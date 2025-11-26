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

#include <ns3/indication-message-helper.h>
#include "ns3/log.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("IndicationMessageHelper");

IndicationMessageHelper::IndicationMessageHelper (IndicationMessageType type, bool isOffline,
                                                  bool reducedPmValues)
    : m_type (type), m_offline (isOffline), m_reducedPmValues (reducedPmValues)
{

  if (!m_offline)
    {
      switch (type)
        {
        case IndicationMessageType::CuUp:
          m_cuUpValues = Create<OCuUpContainerValues> ();
          break;

        case IndicationMessageType::CuCp:
          m_cuCpValues = Create<OCuCpContainerValues> ();
          m_msgValues.m_cellObjectId = "NRCellCU";
          break;

        case IndicationMessageType::Du:
          m_duValues = Create<ODuContainerValues> ();
          break;

        case IndicationMessageType::eNB:
          m_enbValues = Create<eNBContainerValues> ();
          break;
        case IndicationMessageType::gNB:
          m_gnbValues = Create<gNBContainerValues> ();
          break;
        default:

          break;
        }
    }
}

void
IndicationMessageHelper::FillBaseCuUpValues (std::string plmId)
{
  NS_ABORT_MSG_IF (m_type != IndicationMessageType::CuUp, "Wrong function for this object");
  m_cuUpValues->m_plmId = plmId;
  m_msgValues.m_pmContainerValues = m_cuUpValues;
}

void
IndicationMessageHelper::FillBaseCuCpValues (uint16_t numActiveUes)
{
  //NS_ABORT_MSG_IF (m_type != IndicationMessageType::CuCp, "Wrong function for this object");
  m_cuCpValues->m_numActiveUes = numActiveUes;
  m_msgValues.m_pmContainerValues = m_cuCpValues;
}

// update 1111
void
IndicationMessageHelper::FillBaseeNBValues (std::string plmId,uint16_t numActiveUes)
{
  NS_ABORT_MSG_IF (m_type != IndicationMessageType::eNB, "Wrong function for this object");
  m_enbValues->m_plmId = plmId;
  m_enbValues->m_numActiveUes = numActiveUes;
  m_msgValues.m_pmContainerValues = m_enbValues;
}


IndicationMessageHelper::~IndicationMessageHelper ()
{
}
//update 1029
Ptr<KpmIndicationMessage>
IndicationMessageHelper::CreateIndicationMessage (const std::string &targetType)
{
  // 1029 based on format type the object of KpmIndicationMessage Create
  E2SM_KPM_IndicationMessage_FormatType format_type;

  if (targetType == "cell" ) {
      format_type = E2SM_KPM_INDICATION_MESSAGE_FORMART1;
  }  else if (targetType == "ue") {
      format_type = E2SM_KPM_INDICATION_MESSAGE_FORMART2; // FORMAT2
  } else {
      NS_LOG_WARN("Unknown target type: " << targetType << ", defaulting to UE (Format3)");
      format_type = E2SM_KPM_INDICATION_MESSAGE_FORMART3;
  }
/*
  // 1115 For Debug
  Ptr<MeasurementItemList> cellItems = m_msgValues.m_cellMeasurementItems;
  if (!cellItems)
  {
    NS_LOG_DEBUG("Empty MeasurementItemList For Cell");
  }
  else
  {
    auto items = cellItems->GetItems();
    NS_LOG_INFO("Number of items = " << items.size());

    for (const auto &mesItem : items)
    {
      auto v = mesItem->GetValue();   // pmType, pmVal 들어있는 struct

      // ----- 이름(label) 꺼내기 -----
      std::string name = "UNKNOWN";
      if (v.pmType.present == MeasurementType_PR_measName &&
          v.pmType.choice.measName.buf != nullptr &&
          v.pmType.choice.measName.size > 0)
      {
        name.assign(
            reinterpret_cast<char*>(v.pmType.choice.measName.buf),
            v.pmType.choice.measName.size);
      }

      NS_LOG_INFO("  Measurement name = " << name);

      // ----- 값(value) 꺼내기 -----
      switch (v.pmVal.present)
      {
        case MeasurementValue_PR_valueInt:
          NS_LOG_INFO("  Measurement value (INT)  = " << v.pmVal.choice.valueInt);
          break;

        case MeasurementValue_PR_valueReal:
          NS_LOG_INFO("  Measurement value (REAL) = " << v.pmVal.choice.valueReal);
          break;

        case MeasurementValue_PR_noValue:
          NS_LOG_INFO("  Measurement value = <noValue>");
          break;

        case MeasurementValue_PR_valueRRC:
          NS_LOG_INFO("  Measurement value = <RRC struct pointer>");
          break;

        default:
          NS_LOG_INFO("  Measurement value = <unknown type: " << v.pmVal.present << ">");
          break;
      }
    }
}
*/

  return Create<KpmIndicationMessage> (m_msgValues, format_type);
}



} // namespace ns3