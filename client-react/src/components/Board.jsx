import React, {} from 'react';
import Cell from './Cell';
const Board = (props) =>{ 
    return(
        <div id="board">
            <div className="row">
                <Cell num="0" />
                <Cell num="1" />
                <Cell num="2" />
            </div>
            <div className="row">
                <Cell num="3" />
                <Cell num="4" />
                <Cell num="5" />
            </div>
            <div className="row">
                <Cell num="6" />
                <Cell num="7" />
                <Cell num="8" />
            </div>
        </div>
    )

}
export default Board;