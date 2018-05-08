open Jest;
open Level;
open Types;
open Rationale;

let blankWorld = LevelBuilder.makeBlankWorld("test");
let waterTile = { tile: WATER, state: EMPTY };

let playerAt = (x, y) => PLAYER({name:"test", stats: { health: 10, speed: 1.0, position: 0. }, gold: 5, location: (x, y)});
let intialPlayer = { name:"test", stats: { health: 10, speed: 1.0, position: 0.}, gold: 5, location: (0, 0) };
let nfPlayer = { name:"test", stats: { health: 10, speed: 1.0, position: 0. }, gold: 5, location: (9, 9) };

describe("Level.modify", () => {
  test("Can modify a tile", (_) => {
    open Expect;
    let modifiedLevel =
      blankWorld
      |> Level.modifyTile(0, 0, waterTile);
    
      expect(modifiedLevel.map |> List.hd |> List.hd |> (i => i.tile)) |> toBe(WATER);
  });
});

describe("Level.findPlayer", () => {
  test("Finds the player", (_) => {
    open Expect;
    let player =
      blankWorld
      |> Level.modifyTile(0, 0, {tile: GROUND, state: playerAt(0, 0)}) 
      |> Level.findPlayer;
    
      expect(Rationale.Option.isSome(player)) |> toEqual(true);
  });

  test("Returns empty when there is no player", (_) => {
    open Expect;
    let player = blankWorld |> Level.findPlayer;
    
      expect(Rationale.Option.isNone(player)) |> toEqual(true);
  });
});

describe("Level.setPlayerLocation", () => {
  test("Moves the player when the destination is valid", (_) => {
    open Expect;
    let result =
      blankWorld
      |> Level.modifyTile(0, 0, { tile: GROUND, state: playerAt(0, 0)})
      |> Level.setPlayerLocation(0, 1);
    
      expect(Result.isOk(result)) |> toEqual(true);
  });

  test("Returns an error when there is no player", (_) => {
    open Expect;
    let result =
      Level.setPlayerLocation(0, 1, blankWorld);
    
      expect(Result.isError(result)) |> toEqual(true);
      
  });

  test("Returns an error when the target location is a wall", (_) => {
    open Expect;
    let result =
      blankWorld
      |> Level.modifyTile(0, 0, { tile: GROUND, state: playerAt(0, 0)}) 
      |> Level.modifyTile(0, 1, { tile: WALL, state: EMPTY }) 
      |> Level.setPlayerLocation(0, 1);
    
      expect(Result.isError(result)) |> toEqual(true);
  });

  test("Moves the player can move when the target is water", (_) => {
    open Expect;
    let result =
      blankWorld
      |> Level.modifyTile(0, 0, { tile: GROUND, state: playerAt(0, 0)})
      |> Level.modifyTile(0, 1, { tile: WATER, state: EMPTY }) 
      |> Level.setPlayerLocation(0, 1);
    
      expect(Result.isOk(result)) |> toEqual(true);
  });
});


describe("Level.movePlayer", () => {
  test("Moves the player when the destination is valid", (_) => {
    open Expect;
    let result =
      blankWorld
      |> Level.modifyTile(0, 0, { tile: GROUND, state: playerAt(0, 0)})
      |> Level.movePlayer(0, 1);
    
      switch result {
      | Ok(level) => expect(Level.getPlace(0, 1, level.map) |> Rationale.Option.default(waterTile) |> Level.isPlayer) |>  toEqual(true);
      | _ => failwith("Move should succeed")
      };
  });

  test("Updates the player when the destination is valid", (_) => {
    open Expect;
    let result =
      blankWorld
      |> Level.modifyTile(0, 0, { tile: GROUND, state: playerAt(0, 0)})
      |> Level.movePlayer(0, 1);
    
      /* expect(Result.isOk(result)) |> toEqual(true); */
      switch result {
      | Ok(level) => 
        expect(Level.findPlayer(level) |> Rationale.Option.default(nfPlayer) |> pl => pl.location) |>  toEqual((0, 1));
      | _ => failwith("Move should succeed")
      };
  });

  test("Moves the player forward twice, when requested twice", (_) => {
    open Expect;
    let result =
      blankWorld
      |> Level.modifyTile(0, 0, { tile: GROUND, state: playerAt(0, 0)})
      |> Level.movePlayer(0, 1)
      |> Rationale.Result.getOk |> Rationale.Option.default(blankWorld)
      |> Level.movePlayer(0, 1);
    
      /* expect(Result.isOk(result)) |> toEqual(true); */
      switch result {
      | Ok(level) => {
          let ply = Level.findPlayer(level) |> Rationale.Option.default(nfPlayer);
          
          expect(ply.location) |> toEqual((0, 2));
         };
      | _ => failwith("Move should succeed")
      };
  });
});
