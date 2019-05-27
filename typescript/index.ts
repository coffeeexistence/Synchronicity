import Rx = require("rxjs");
import { map, filter, mergeMap, take, mapTo } from "rxjs/operators";
import createAndReturnVirtualPortStream, {
  MidiMessage
} from "./createAndReturnVirtualPortStream";

const messageTypes = {
  ON: 144,
  OFF: 128
};
const midiPortName = "Light Strip";
const lightCount = 12;

const midiChannelStream = createAndReturnVirtualPortStream(midiPortName);

const noteOnStream: Rx.Observable<MidiMessage> = midiChannelStream.pipe(
  filter(midiMessage => midiMessage.type === messageTypes.ON)
);

const noteOffStream: Rx.Observable<MidiMessage> = midiChannelStream.pipe(
  filter(midiMessage => midiMessage.type === messageTypes.OFF)
);

enum ColorPreset {
  BLANK = 0,
  LIGHT_RED = 1
}

type LightChangeInstruction = {
  startPixel: number;
  endPixel: number;
  colorPreset: ColorPreset;
};

type LightChangeInstructions = {
  onNoteStart: LightChangeInstruction;
  onNoteEnd: LightChangeInstruction;
};

const getLightChangeInstructionsForNoteMessage = (
  midiMessage: MidiMessage
): LightChangeInstructions => {
  const onNoteStart = {
    startPixel: 0,
    endPixel: 20,
    colorPreset: ColorPreset.LIGHT_RED
  };
  return {
    onNoteStart,
    onNoteEnd: { ...onNoteStart, colorPreset: ColorPreset.BLANK }
  };
};

const lightChangeStream: Rx.Observable<
  LightChangeInstruction
> = noteOnStream.pipe(
  mergeMap((midiMessage: MidiMessage) => {
    const { onNoteStart, onNoteEnd } = getLightChangeInstructionsForNoteMessage(
      midiMessage
    );
    const onNoteStartObservable = Rx.of(onNoteStart);
    // onNoteEnd is deferred until a noteOffStream message with the same note is detected
    const onNoteEndObservable = noteOffStream.pipe(
      filter(endMessaage => endMessaage.note === midiMessage.note),
      take(1),
      mapTo(onNoteEnd)
    );

    return Rx.merge(onNoteStartObservable, onNoteEndObservable);
  })
);

lightChangeStream.subscribe(message => {
  console.log("lightChangeStream", message);
});
console.log("subscribed to midi event stream");
