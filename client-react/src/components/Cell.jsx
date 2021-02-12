import React from 'react';
const Cell = (props) =>{
    return <span className="cell" id={`cell-${props.num}`}>
        {props.num}
    </span>
}
export default Cell;