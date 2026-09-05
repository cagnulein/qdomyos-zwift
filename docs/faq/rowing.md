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

## How can I show my rowing pace per 500 m in QZ?

Open **QZ Settings > Tiles** and enable **Pace Last 500m**.

The tile shows rowing pace as the time needed to cover 500 metres, the standard pace format commonly used for rowing.
