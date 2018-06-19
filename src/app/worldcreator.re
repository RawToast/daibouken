open Types;

module CsvWorldBuilder: WorldCreator = {
  let makeExit = score => { tile: EXIT(int_of_string((score))), state: Empty, tileEffect: NoEff };

  let tail = Js.String.sliceToEnd(~from=1);
  let head = Js.String.charAt(0);

  let makeStairs = str => {
    let id = str |> head |> int_of_string;
    let level = str |> tail;

    let link = { id: id, level: level};
    { tile: STAIRS(link), state: Empty, tileEffect: NoEff };
  };

  let makeTile = (str) => {
    switch (Js.String.charAt(0, str)) {
    | "." => { tile: GROUND, state: Empty, tileEffect: NoEff }
    | "w" => { tile: WATER, state: Empty, tileEffect: NoEff }
    | "#" => { tile: WALL, state: Empty, tileEffect: NoEff }
    | "e" => str |> tail |> makeExit
    | "/" => str |> tail |> makeStairs
    | _ => { tile: WALL, state: Empty, tileEffect: NoEff }
    };
  };

  let createOccupier: (string, place) => place = (str, place) => {
    if(Rationale.RList.any(k => str == k, ["+", ":", "g", ";"])) {
        switch str {
        | "+" => { ... place, tileEffect: Heal(2) }
        | ":" => { ... place, tileEffect: Trap(2) }
        | ";" => { ... place, tileEffect: Snare(2.) }
        | "g" => { ... place, tileEffect: Gold(3) }
        | _ => place
        }
    } else Enemy.Enemies.addEnemy(str, place);
  };

  let buildPlace = str => { 
    let spliced = str  |> Js.String.split("|") |> Array.to_list; 

    switch (List.length(spliced)) {
    | 1 => spliced |> List.hd |> makeTile
    | 2 => spliced |> List.hd |> makeTile |> createOccupier(List.nth(spliced, 1), _)
    | _ => makeTile(".")
    };
  };

  let buildArea = (str) => str 
      |> Js.String.split("\n") 
      |> Array.to_list
      |> List.map(col =>
        col |> Js.String.split(",") 
            |> Array.to_list
            |> List.map(Js.String.trim)
            |> List.map(buildPlace))
      |> List.filter(x => List.length(x) != 0)
      |> List.rev;

  let buildLevel = (name, areaStr) => { name: name, map: buildArea(areaStr) };

  let loadLevel = (directory, file) =>
    Node.Fs.readFileAsUtf8Sync(directory ++ "/" ++ file) |> buildLevel(Js.String.slice(~from=0, ~to_=(Js.String.indexOf(".", file)), file));

  let loadWorld = (initial, directory) => {   
    let levels = directory 
      |> Node.Fs.readdirSync
      |> Array.to_list 
      |> List.map(file => loadLevel(directory, file));

    { current: initial, levels: levels };
  };

  let loadWorldAsync = (initial, names) => {

    let levelNames = Js.String.split(",", names) |> Array.to_list;

    let prom = (name) => Js.Promise.(
      Fetch.fetch("/world/" ++ name ++ ".csv")
        |> then_(Fetch.Response.text)
        |> then_(text => buildLevel(name, text) |> resolve)
    );

    levelNames 
      |> List.map(prom) 
      |> Array.of_list 
      |> Js.Promise.all 
      |> Js.Promise.then_(lvl => lvl |> Array.to_list |> lvs => {current: initial, levels: lvs} |> Js.Promise.resolve);
  };
};

