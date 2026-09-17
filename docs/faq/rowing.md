# Rowing

## Can I use QZ as a virtual rower when my rowing machine cannot broadcast usable data?

Yes. QZ can act as a virtual FTMS rower and provide generated rowing data to a training app such as Kinomap even when the original rowing machine cannot expose usable fitness data.

For a basic setup:

1. Enable QZ's virtual rower functionality.
2. In the training app, pair QZ as an **FTMS rower**.
3. For Kinomap, keep the **PM5** compatibility option disabled unless you are specifically using a Concept2/PM5 setup.
4. Start with the speed and watt offsets set to **0**.
5. Use the **Target Watt** tile on the QZ dashboard to change the generated effort. Changing the target watts changes the simulated rowing metrics sent by QZ.

If Bluetooth discovery is unreliable with both apps on the same device, running QZ and the training app on separate devices can simplify the connection.

This setup was explicitly confirmed working in a support case where the original rower could not provide usable data to the training app.

## EXR connects to QZ as a PM5 but does not receive usable rowing data or disconnects. What should I try?

If you are bridging a non-Concept2 rower through QZ to EXR, use QZ's standard virtual FTMS rower rather than PM5 emulation:

1. Enable the **Virtual Rower** in QZ.
2. Disable the **PM5** virtual-rower compatibility option.
3. Restart the virtual connection if necessary, then pair EXR with the QZ rower again.

With PM5 mode enabled, QZ advertises Concept2-specific PM5 services and a PM5-style device identity. With PM5 mode disabled, the virtual rower exposes the standard FTMS rower service instead.

In a confirmed support case, EXR initially connected to QZ as `PM5 430000000` but did not receive usable distance/speed/pace data and the virtual connection dropped. Disabling PM5 while leaving the virtual rower enabled made EXR discover the rower as QZ and all rowing data worked correctly.

Use PM5 mode only when you specifically need Concept2/PM5 compatibility.

## How can I show my rowing pace per 500 m in QZ?

Open **QZ Settings > Tiles** and enable **Pace Last 500m**.

The tile shows rowing pace as the time needed to cover 500 metres, the standard pace format commonly used for rowing.
