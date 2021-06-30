/*
 * Copyright (c) 2014 VMware, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ndis.h>
#include <netiodef.h>
#include <intsafe.h>
#include <ntintsafe.h>
#include <ntstrsafe.h>

#include "Types.h"

#include "..\include\OvsDpInterface.h"

#include "Util.h"
#include "Netlink/NetlinkError.h"
#include "Netlink/Netlink.h"
#include "Netlink/NetlinkProto.h"
#include "..\include\OvsDpInterfaceExt.h"
#include "..\include\OvsDpInterfaceCtExt.h"
#include "DpInternal.h"
