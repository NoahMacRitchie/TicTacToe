import React, { useState, useEffect } from 'react';

import Board from './Board'
import electron from 'electron';
const Game = () => {
    const [myTurn, setMyTurn] = useState(false);
    const [board, setBoard] = useState(new Array(9).fill(null));
    const [myTeam, setTeam] = useState(null);
    
    useEffect(()=> {
        electron.ipcRenderer.on('notify-team', (event, data) => {
            console.log("got team!");
            console.log(data);
            setTeam(data[1]); 
        });
        // ipcRenderer.on('successful-move', (event, data) => {
        //     console.log("got move");
        //     console.log(data);
        //     let newBoard = [...board];
        //     newBoard[data[1]] =  myTurn ? myTeam : getOpposingTeam(myTeam); 
            
            
        // });


    }, [])

    return (
        <Board/>
    )
}

export default Game;

const getOpposingTeam = (team) => {
    return team == "X" ? "O" : "X";
}