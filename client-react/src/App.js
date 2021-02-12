import React from "react";
import { Link } from "react-router-dom";
import Game from "./components/Game";
import "./App.css"
export default class App extends React.Component {

  
  render() {
    let connection_status = "Connected...";
    return (
      <div>
        <h1>TicTacToe</h1>
        <div><h3 id="team-name"></h3></div>
        <Game />
        <div id="status">Status: {connection_status}</div>

      </div>
    );
  }
}
