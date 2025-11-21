#!/usr/bin/env node

import { Server } from '@modelcontextprotocol/sdk/server/index.js';
import { StdioServerTransport } from '@modelcontextprotocol/sdk/server/stdio.js';
import {
  CallToolRequestSchema,
  ListToolsRequestSchema,
} from '@modelcontextprotocol/sdk/types.js';
import * as fs from 'fs';
import * as path from 'path';

// HCI Packet Types
const HCI_COMMAND = 0x01;
const HCI_ACL_DATA = 0x02;
const HCI_EVENT = 0x04;

// ATT Opcodes
const ATT_WRITE_COMMAND = 0x52;
const ATT_WRITE_REQUEST = 0x12;
const ATT_HANDLE_VALUE_NOTIFICATION = 0x1B;

class HCISnoopParser {
  constructor(filePath) {
    this.filePath = filePath;
    this.packets = [];
  }

  parse() {
    const buffer = fs.readFileSync(this.filePath);

    // Verify btsnoop header
    const magic = buffer.toString('ascii', 0, 8);
    if (magic !== 'btsnoop\0') {
      throw new Error('Invalid btsnoop file format');
    }

    const version = buffer.readUInt32BE(8);
    const datalink = buffer.readUInt32BE(12);

    let offset = 16; // Skip file header

    while (offset < buffer.length) {
      try {
        const packet = this.parsePacketRecord(buffer, offset);
        if (packet) {
          this.packets.push(packet);
          offset = packet.nextOffset;
        } else {
          break;
        }
      } catch (e) {
        console.error(`Error parsing packet at offset ${offset}:`, e.message);
        break;
      }
    }

    return this.packets;
  }

  parsePacketRecord(buffer, offset) {
    if (offset + 25 > buffer.length) {
      return null;
    }

    const originalLength = buffer.readUInt32BE(offset);
    const includedLength = buffer.readUInt32BE(offset + 4);
    const flags = buffer.readUInt32BE(offset + 8);
    const drops = buffer.readUInt32BE(offset + 12);
    const timestamp = buffer.readBigUInt64BE(offset + 16);

    offset += 24;

    if (offset + includedLength > buffer.length) {
      return null;
    }

    const data = buffer.slice(offset, offset + includedLength);

    // Parse HCI packet
    const direction = (flags & 0x01) ? 'received' : 'sent';
    const hciPacket = this.parseHCIPacket(data, direction);

    return {
      originalLength,
      includedLength,
      flags,
      direction,
      timestamp: Number(timestamp),
      data,
      hciPacket,
      nextOffset: offset + includedLength
    };
  }

  parseHCIPacket(data, direction) {
    if (data.length === 0) return null;

    const packetType = data[0];

    if (packetType === HCI_ACL_DATA) {
      return this.parseACLData(data, direction);
    } else if (packetType === HCI_EVENT) {
      return this.parseHCIEvent(data);
    } else if (packetType === HCI_COMMAND) {
      return this.parseHCICommand(data);
    }

    return { type: 'unknown', packetType, data };
  }

  parseACLData(data, direction) {
    if (data.length < 5) return null;

    // HCI ACL header: 1 byte type + 2 bytes handle + 2 bytes length
    const handle = data.readUInt16LE(1) & 0x0FFF;
    const pbFlag = (data.readUInt16LE(1) >> 12) & 0x03;
    const bcFlag = (data.readUInt16LE(1) >> 14) & 0x03;
    const dataLength = data.readUInt16LE(3);

    if (data.length < 5 + dataLength) return null;

    // L2CAP header: 2 bytes length + 2 bytes channel ID
    if (dataLength < 4) return null;

    const l2capLength = data.readUInt16LE(5);
    const l2capChannel = data.readUInt16LE(7);

    // ATT channel is 0x0004
    if (l2capChannel === 0x0004 && dataLength >= 5) {
      return this.parseATTPacket(data.slice(9), handle, direction);
    }

    return {
      type: 'acl',
      handle,
      direction,
      l2capChannel,
      data: data.slice(5)
    };
  }

  parseATTPacket(data, handle, direction) {
    if (data.length === 0) return null;

    const opcode = data[0];

    let attType = 'unknown';
    let characteristicHandle = null;
    let payload = null;

    if (opcode === ATT_WRITE_COMMAND || opcode === ATT_WRITE_REQUEST) {
      attType = opcode === ATT_WRITE_COMMAND ? 'write_command' : 'write_request';
      if (data.length >= 3) {
        characteristicHandle = data.readUInt16LE(1);
        payload = data.slice(3);
      }
    } else if (opcode === ATT_HANDLE_VALUE_NOTIFICATION) {
      attType = 'notification';
      if (data.length >= 3) {
        characteristicHandle = data.readUInt16LE(1);
        payload = data.slice(3);
      }
    } else {
      payload = data.slice(1);
    }

    return {
      type: 'att',
      attType,
      opcode,
      handle,
      characteristicHandle,
      direction,
      payload,
      fullPacket: data
    };
  }

  parseHCIEvent(data) {
    if (data.length < 3) return null;

    const eventCode = data[1];
    const paramLength = data[2];

    return {
      type: 'event',
      eventCode,
      params: data.slice(3, 3 + paramLength)
    };
  }

  parseHCICommand(data) {
    if (data.length < 4) return null;

    const opcode = data.readUInt16LE(1);
    const paramLength = data[3];

    return {
      type: 'command',
      opcode,
      params: data.slice(4, 4 + paramLength)
    };
  }

  filterWriteCommands() {
    return this.packets.filter(p =>
      p.hciPacket?.type === 'att' &&
      p.hciPacket.direction === 'sent' &&
      (p.hciPacket.attType === 'write_command' || p.hciPacket.attType === 'write_request')
    );
  }

  filterNotifications() {
    return this.packets.filter(p =>
      p.hciPacket?.type === 'att' &&
      p.hciPacket.direction === 'received' &&
      p.hciPacket.attType === 'notification'
    );
  }

  removeBLEHeader(payload) {
    // Remove the first 11 bytes (BLE header) as documented in CLAUDE.md
    if (payload && payload.length > 11) {
      return payload.slice(11);
    }
    return payload;
  }

  generateCppArray(packets, removeBLEHeader = true) {
    const arrays = [];

    packets.forEach((packet, index) => {
      if (packet.hciPacket?.payload) {
        let payload = packet.hciPacket.payload;

        if (removeBLEHeader) {
          payload = this.removeBLEHeader(payload);
        }

        if (payload && payload.length > 0) {
          const hexBytes = Array.from(payload)
            .map(b => `0x${b.toString(16).padStart(2, '0')}`)
            .join(', ');

          arrays.push({
            index,
            timestamp: packet.timestamp,
            handle: packet.hciPacket.characteristicHandle,
            code: `uint8_t data${index}[] = {${hexBytes}};`,
            length: payload.length,
            originalLength: packet.hciPacket.payload.length
          });
        }
      }
    });

    return arrays;
  }

  analyzeSequence(packets) {
    const sequences = [];
    const payloads = packets.map(p => p.hciPacket?.payload).filter(p => p);

    // Look for repeating patterns
    const payloadHashes = payloads.map(p => p.toString('hex'));
    const uniquePayloads = [...new Set(payloadHashes)];

    // Find where repetition starts (likely sendPoll boundary)
    let repeatBoundary = -1;
    for (let i = 0; i < payloads.length - 5; i++) {
      const pattern = payloadHashes.slice(i, i + 3).join('|');
      const remaining = payloadHashes.slice(i + 3).join('|');
      if (remaining.includes(pattern)) {
        repeatBoundary = i;
        break;
      }
    }

    return {
      totalPackets: packets.length,
      uniquePayloads: uniquePayloads.length,
      repeatBoundaryIndex: repeatBoundary,
      initFramesCount: repeatBoundary > 0 ? repeatBoundary : packets.length,
      sendPollFramesCount: repeatBoundary > 0 ? packets.length - repeatBoundary : 0
    };
  }
}

// Create MCP server
const server = new Server(
  {
    name: 'hci-sniffer',
    version: '1.0.0',
  },
  {
    capabilities: {
      tools: {},
    },
  }
);

// List available tools
server.setRequestHandler(ListToolsRequestSchema, async () => {
  return {
    tools: [
      {
        name: 'parse_hci_snoop',
        description: 'Parse Android HCI snoop log file (btsnoop_hci.log) and extract BLE packets',
        inputSchema: {
          type: 'object',
          properties: {
            file_path: {
              type: 'string',
              description: 'Absolute path to the btsnoop_hci.log file',
            },
            filter: {
              type: 'string',
              enum: ['all', 'writes', 'notifications'],
              description: 'Filter packets: all, writes (sent to device), or notifications (received from device)',
              default: 'writes',
            },
          },
          required: ['file_path'],
        },
      },
      {
        name: 'generate_cpp_code',
        description: 'Generate C++ code arrays from HCI snoop file for ProForm device implementation',
        inputSchema: {
          type: 'object',
          properties: {
            file_path: {
              type: 'string',
              description: 'Absolute path to the btsnoop_hci.log file',
            },
            remove_ble_header: {
              type: 'boolean',
              description: 'Remove the first 11 bytes (BLE header) from each packet',
              default: true,
            },
            auto_split: {
              type: 'boolean',
              description: 'Automatically split into initialization and sendPoll frames',
              default: true,
            },
          },
          required: ['file_path'],
        },
      },
      {
        name: 'analyze_packet_sequence',
        description: 'Analyze packet sequence to identify initialization vs sendPoll boundary',
        inputSchema: {
          type: 'object',
          properties: {
            file_path: {
              type: 'string',
              description: 'Absolute path to the btsnoop_hci.log file',
            },
          },
          required: ['file_path'],
        },
      },
    ],
  };
});

// Handle tool calls
server.setRequestHandler(CallToolRequestSchema, async (request) => {
  const { name, arguments: args } = request.params;

  try {
    if (name === 'parse_hci_snoop') {
      const parser = new HCISnoopParser(args.file_path);
      parser.parse();

      let packets;
      if (args.filter === 'writes') {
        packets = parser.filterWriteCommands();
      } else if (args.filter === 'notifications') {
        packets = parser.filterNotifications();
      } else {
        packets = parser.packets;
      }

      const summary = packets.map((p, i) => ({
        index: i,
        direction: p.direction,
        type: p.hciPacket?.type,
        attType: p.hciPacket?.attType,
        handle: p.hciPacket?.characteristicHandle,
        payloadLength: p.hciPacket?.payload?.length || 0,
        timestamp: p.timestamp,
      }));

      return {
        content: [
          {
            type: 'text',
            text: JSON.stringify({
              file: args.file_path,
              totalPackets: parser.packets.length,
              filteredPackets: packets.length,
              filter: args.filter || 'writes',
              packets: summary,
            }, null, 2),
          },
        ],
      };
    }

    if (name === 'generate_cpp_code') {
      const parser = new HCISnoopParser(args.file_path);
      parser.parse();
      const writes = parser.filterWriteCommands();

      const analysis = parser.analyzeSequence(writes);
      const arrays = parser.generateCppArray(writes, args.remove_ble_header !== false);

      let output = `// Generated from: ${path.basename(args.file_path)}\n`;
      output += `// Total write packets: ${arrays.length}\n`;

      if (args.auto_split && analysis.repeatBoundaryIndex > 0) {
        output += `// Initialization frames: ${analysis.initFramesCount}\n`;
        output += `// SendPoll frames: ${analysis.sendPollFramesCount}\n\n`;

        output += `// === INITIALIZATION FRAMES (for btinit method) ===\n\n`;
        arrays.slice(0, analysis.initFramesCount).forEach(arr => {
          output += `${arr.code}\n`;
          output += `writeCharacteristic(data${arr.index}, sizeof(data${arr.index}), QStringLiteral("init"), false, true);\n\n`;
        });

        output += `\n// === SENDPOLL FRAMES (for sendPoll method) ===\n\n`;
        const sendPollFrames = arrays.slice(analysis.initFramesCount);
        sendPollFrames.forEach((arr, idx) => {
          output += `case ${idx}:\n`;
          output += `    ${arr.code}\n`;
          output += `    writeCharacteristic(data${arr.index}, sizeof(data${arr.index}), QStringLiteral("poll"), false, true);\n`;
          output += `    break;\n`;
        });
      } else {
        arrays.forEach(arr => {
          output += `${arr.code}\n`;
          output += `writeCharacteristic(data${arr.index}, sizeof(data${arr.index}), QStringLiteral("write"), false, true);\n\n`;
        });
      }

      return {
        content: [
          {
            type: 'text',
            text: output,
          },
        ],
      };
    }

    if (name === 'analyze_packet_sequence') {
      const parser = new HCISnoopParser(args.file_path);
      parser.parse();
      const writes = parser.filterWriteCommands();

      const analysis = parser.analyzeSequence(writes);

      return {
        content: [
          {
            type: 'text',
            text: JSON.stringify(analysis, null, 2),
          },
        ],
      };
    }

    throw new Error(`Unknown tool: ${name}`);
  } catch (error) {
    return {
      content: [
        {
          type: 'text',
          text: `Error: ${error.message}`,
        },
      ],
      isError: true,
    };
  }
});

// Start server
async function main() {
  const transport = new StdioServerTransport();
  await server.connect(transport);
  console.error('HCI Sniffer MCP server running on stdio');
}

main().catch(console.error);
