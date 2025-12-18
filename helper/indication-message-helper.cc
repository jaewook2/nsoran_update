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
         // To be deleted it is not used 
        case IndicationMessageType::eNB:
          m_msgValues.m_cellObjectId = "eNB";
          break;
        case IndicationMessageType::gNB:
          m_msgValues.m_cellObjectId = "gNB";
          break;
        default:

          break;
        }
    }
}



IndicationMessageHelper::~IndicationMessageHelper ()
{
}

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

  return Create<KpmIndicationMessage> (m_msgValues, format_type);
}



} // namespace ns3