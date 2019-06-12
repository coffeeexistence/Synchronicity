import Rx = require("rxjs");
import { map, filter, mergeMap, take, mapTo } from "rxjs/operators";
import SerialPort = require("serialport");
import createAndReturnVirtualPortStream, {
  MidiMessage
} from "./createAndReturnVirtualPortStream";

type LightChangeInstruction = {
  startPixel: number;
  endPixel: number;
  colorPreset: ColorPreset;
};

const port = new SerialPort("/dev/cu.SLAB_USBtoUART", {
  baudRate: 115200,
  databits: 8,
  autoOpen: false
});

let lastSentDate = 0;
const timeDiffs = [];

port.on("data", data => {
  // const timeDiff = Date.now() - lastSentDate;
  // timeDiffs.unshift(timeDiff);
  // if (timeDiffs.length > 5) {
  //   timeDiffs.pop();
  // }
  // const timeDiffAverage =
  //   timeDiffs.reduce((prev, curr) => prev + curr, 0) / timeDiffs.length;

  // console.log(data.toString(), `avg-diff: ${timeDiffAverage}ms`);
  console.log(data.toString());
});

const openPort = () =>
  new Promise((resolve, reject) => {
    port.open(error => {
      if (error) {
        console.error("Error while opening the port " + error);
        reject(error);
      } else {
        resolve();
      }
    });
  });

const sendLightChangeInstruction = (instruction: LightChangeInstruction) => {
  // Using strings because ints have horrible issues with the serialport library
  let instructions = [
    instruction.startPixel,
    instruction.endPixel,
    instruction.colorPreset
  ];
  const instructionString = `${instructions.join("-")}-\n`;
  console.log("string: ", instructionString);
  lastSentDate = Date.now();
  port.write(instructionString, (err, _result) => {
    if (err) {
      console.error("port write error", err);
    }
    // port.write(instructionString, (err, _result) => {
    //   if (err) {
    //     console.error("port write error", err);
    //   }
    // });
  });
};

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

type LightChangeInstructions = {
  onNoteStart: LightChangeInstruction;
  onNoteEnd: LightChangeInstruction;
};

const pixelCount = 150;
const getLightChangeInstructionsForNoteMessage = (
  midiMessage: MidiMessage
): LightChangeInstructions => {
  const relativeNote = midiMessage.note % 12;
  const multiplier = Math.floor(pixelCount / 12);
  const onNoteStart = {
    startPixel: multiplier * relativeNote,
    endPixel: multiplier * relativeNote + multiplier,
    colorPreset: 1 + Math.floor(Math.random() * 5)
  };
  if (onNoteStart.startPixel > pixelCount - 1) {
    throw new Error("startPixel too high, value:" + onNoteStart.startPixel);
  }
  if (onNoteStart.endPixel > pixelCount - 1) {
    throw new Error("endPixel too high, value:" + onNoteStart.endPixel);
  }
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

const initializeMainApplication = async () => {
  await openPort();
  lightChangeStream.subscribe(lightChangeInstruction => {
    console.log("lightChangeStream", lightChangeInstruction);
    sendLightChangeInstruction(lightChangeInstruction);
  });
  console.log("initializeMainApplication complete");
};

initializeMainApplication();
