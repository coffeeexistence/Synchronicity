import midi = require("midi");
import Rx = require("rxjs");

type MidiMessage = {
  type: number;
  note: number;
  velocity: number;
};

export default (streamName: string): Rx.Subject<MidiMessage> => {
  const stream: Rx.Subject<MidiMessage> = new Rx.Subject();
  const input = new midi.input();
  input.on("message", (_deltaTime, [type, note, velocity]) =>
    stream.next({ type, note, velocity })
  );
  input.openVirtualPort(streamName);
  return stream;
};
