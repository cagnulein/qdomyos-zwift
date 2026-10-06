# Crashes and unexpected exits

## QZ freezes, closes, or crashes during use

### Provide first

- Confirm explicitly **which app crashes**: QZ or the training app used with it.
- The device and operating system running QZ.
- The fitness machine/trainer model and any additional sensors connected to QZ, such as a heart-rate monitor.
- The exact trigger: for example, immediately after launch, when the machine connects, when pedaling starts, when a workout starts, or when a control command is received.
- Whether the crash also happens when QZ is used without the third-party training app.

### If still unclear

- Provide the relevant QZ settings or profile screenshots, especially virtual-device and integration settings.
- Disable unrelated integrations/connections and reproduce the smallest configuration that still crashes.
- Enable QZ Debug Log and reproduce the crash if possible. If the app exits before the normal log-email flow can complete, provide any available platform crash report together with the last actions performed.

### Why it matters

- In a multi-app setup, identifying the application that actually exits is the first branching point; otherwise diagnostics can target the wrong software.
- The exact trigger narrows the problem to discovery, device parsing, metric updates, virtual-device advertising, or control handling.
- Reproducing without the third-party app separates a QZ/device problem from an interaction between QZ and another application.
- Reducing the setup before collecting deeper logs makes the resulting evidence substantially easier to interpret.
