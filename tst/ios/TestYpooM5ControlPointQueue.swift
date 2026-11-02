import CoreBluetooth
import Foundation

// Run with: python3 tools/test_ypoo_m5_ios_control.py (macOS with Xcode).
@main
struct TestYpooM5ControlPointQueue {
    static func main() {
        precondition(forwardedTreadmillFTMSFeatures[4] == 0x03) // Speed and incline target support.
        // Keep the Objective-C selector used by lockscreen.mm aligned with Swift.
        let forwarding = #selector(virtualtreadmill_zwift.enableControlPointForwarding(speedRange:inclineRange:))
        precondition(NSStringFromSelector(forwarding) == "enableControlPointForwardingWithSpeedRange:inclineRange:")
        let queue = TreadmillControlPointWriteQueue()
        let start = Data([0x07])
        let speed = Data([0x02, 0x78, 0x00])
        let incline = Data([0x03, 0x0a, 0x00])
        let stop = Data([0x08, 0x01])

        // Multiple writes in the same second retain their order without timestamps.
        for data in [start, speed, incline, speed] {
            precondition(queue.enqueue(data) == .success)
        }
        for data in [start, speed, incline, speed] {
            precondition(queue.take() == data)
        }
        precondition(queue.take() == nil)

        // Stop must supersede pending starts/targets, even when the queue is full.
        for _ in 0..<16 { precondition(queue.enqueue(start) == .success) }
        precondition(queue.enqueue(speed) == .insufficientResources)
        precondition(queue.enqueue(stop) == .success)
        precondition(queue.take() == stop)
        precondition(queue.take() == nil)

        let zeroSpeed = Data([0x02, 0, 0])
        precondition(queue.enqueue(start) == .success)
        precondition(queue.enqueue(incline) == .success)
        precondition(queue.enqueue(zeroSpeed) == .success)
        precondition(queue.take() == zeroSpeed)
        precondition(queue.take() == nil)

        // A disconnect cannot leave a delayed start behind or lose an accepted stop.
        precondition(queue.enqueue(start) == .success)
        queue.clear(keepingStop: true)
        precondition(queue.take() == nil)
        precondition(queue.enqueue(stop) == .success)
        precondition(queue.enqueue(start) == .success)
        queue.clear(keepingStop: true)
        precondition(queue.take() == stop)
        precondition(queue.take() == nil)
        precondition(queue.enqueue(start) == .success)
        queue.clear()
        precondition(queue.take() == nil)

        // Reject malformed values before the Qt FTMS parser indexes their bytes.
        precondition(queue.enqueue(Data()) == .requestNotSupported)
        precondition(queue.enqueue(Data([0xff])) == .requestNotSupported)
        for data in [Data([0x02]), Data([0x03, 0]), Data([0x08]), Data([0x11, 0, 0])] {
            precondition(queue.enqueue(data) == .invalidAttributeValueLength)
        }
        precondition(queue.take() == nil)
        print("YPOO M5 iOS queue tests passed: ordering, stop priority, bounds, disconnects, malformed writes.")
    }
}
