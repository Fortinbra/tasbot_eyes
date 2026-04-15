# Squad Team

> Retro LED firmware project migrating from Raspberry Pi userspace control to Raspberry Pi Pico SDK firmware.

## Coordinator

| Name | Role | Notes |
|------|------|-------|
| Squad | Coordinator | Routes work, enforces handoffs and reviewer gates. |

## Members

| Name | Role | Charter | Status |
|------|------|---------|--------|
| Dr. Light | Lead | `.squad/agents/dr-light/charter.md` | ✅ Active |
| Proto Man | Backend Dev | `.squad/agents/proto-man/charter.md` | ✅ Active |
| Roll | Frontend/UI Dev | `.squad/agents/roll/charter.md` | ✅ Active |
| Mega Man | Tester | `.squad/agents/mega-man/charter.md` | ✅ Active |
| Auto | Platform Dev | `.squad/agents/auto/charter.md` | ✅ Active |
| Scribe | Session Logger | `.squad/agents/scribe/charter.md` | 📋 Silent |
| Ralph | Work Monitor | — | 🔄 Monitor |

## Coding Agent

<!-- copilot-auto-assign: false -->

| Name | Role | Charter | Status |
|------|------|---------|--------|
| @copilot | Coding Agent | — | 🤖 Coding Agent |

### Capabilities

**🟢 Good fit — auto-route when enabled:**
- Bug fixes with clear reproduction steps
- Test coverage gaps with defined expectations
- Build, lint, and documentation maintenance
- Small isolated refactors following existing patterns

**🟡 Needs review — route to @copilot but flag for squad member PR review:**
- Medium features with a clear spec
- Refactors that touch multiple modules but keep existing behavior
- Mechanical SDK or toolchain migrations with explicit acceptance criteria

**🔴 Not suitable — route to squad member instead:**
- Architecture decisions and system design
- Hardware interface changes requiring timing judgment
- Performance-critical rendering paths
- Ambiguous migration work needing coordination

## Project Context

- **Owner:** Fortinbra
- **Project:** tasbot_eyes
- **Stack:** C, CMake, Raspberry Pi Pico SDK, LED matrix firmware
- **Description:** Controls an 8x32 LED array and currently renders animations and GIF data from a Raspberry Pi oriented C codebase.
- **Created:** 2026-04-15
