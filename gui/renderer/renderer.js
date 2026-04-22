/* global $, Chessboard, Chess */
const game = new Chess();
let board = null;
let engineReady = false;
let engineLoaded = false;
let thinking = false;
let lastBestmove = null;
let lastFromSq = null;
let lastToSq = null;
let startFen = null; // null = startpos, string = FEN anchor for position command
let currentPvMove = null; // UCI string of pv[0] during search
let currentScoreText = null; // score string to render on the arrow
let pendingRestart = false; // restart infinite analysis after bestmove arrives
let pendingGo = null; // pending manual go params, fires after bestmove
let selfPlay = false;
let searchStartMs = 0;
let lastCp = null; // latest engine eval in centipawns, from white's POV (mate => ±10000)
let evalHistory = []; // [{ply, cp}] after each played move
let pgnAnalysis = null; // { active, cancel, resolveBestmove }
let loadedGame = { moves: [], startFen: null, headers: {} };
let currentPly = 0;

const $log = document.getElementById('log');
const $status = document.getElementById('status');
const $engineName = document.getElementById('engine-name');
const $engineEval = document.getElementById('engine-eval');
const $pvLine = document.getElementById('pv-line');
const $fen = document.getElementById('fen');
const ids = ['depth', 'seldepth', 'score', 'nodes', 'nps', 'time', 'hashfull', 'tthash'];
const $i = Object.fromEntries(ids.map(k => [k, document.getElementById('i-' + k)]));

function appendLog(line) {
  $log.textContent += line + '\n';
  $log.scrollTop = $log.scrollHeight;
}

function evalToFraction(cp) {
  // cp is from white's POV; mate = ±10000. Map to 0..1 (white's share of bar).
  if (cp === null || cp === undefined) return 0.5;
  if (cp >= 10000) return 1;
  if (cp <= -10000) return 0;
  const c = Math.max(-1000, Math.min(1000, cp));
  return 1 / (1 + Math.exp(-0.004 * c));
}

function formatEvalLabel(cp) {
  if (cp === null || cp === undefined) return '0.0';
  if (cp >= 10000) return 'M';
  if (cp <= -10000) return '-M';
  const p = cp / 100;
  return (p >= 0 ? '+' : '') + p.toFixed(1);
}

function updateEvalBar(cp) {
  const bar = document.getElementById('eval-bar');
  if (!bar) return;
  const frac = evalToFraction(cp);
  const H = bar.clientHeight || 640;
  const whitePx = Math.round(frac * H);
  const blackPx = H - whitePx;
  const orient = bar.dataset.orient;
  const blackEl = document.getElementById('eval-bar-black');
  const whiteEl = document.getElementById('eval-bar-white');
  const mid = document.getElementById('eval-bar-mid');
  const topLbl = document.getElementById('eval-bar-label-top');
  const botLbl = document.getElementById('eval-bar-label-bot');
  whiteEl.style.height = whitePx + 'px';
  blackEl.style.height = blackPx + 'px';
  // mid line sits at the boundary between white and black
  mid.style.bottom = (orient === 'black' ? (H - whitePx - 1) : (whitePx - 1)) + 'px';
  const label = formatEvalLabel(cp);
  // label shown on the losing side (the smaller portion), like lichess
  if (frac >= 0.5) {
    // white is winning — label at bottom (white side when orient=white)
    if (orient === 'white') { botLbl.textContent = label; topLbl.textContent = ''; }
    else                    { topLbl.textContent = label; botLbl.textContent = ''; }
  } else {
    if (orient === 'white') { topLbl.textContent = label; botLbl.textContent = ''; }
    else                    { botLbl.textContent = label; topLbl.textContent = ''; }
  }
}

function setThinking(v) {
  thinking = v;
  const el = document.getElementById('search-indicator');
  if (!el) return;
  el.classList.toggle('searching', !!v);
  el.classList.toggle('idle', !v);
  el.querySelector('.search-label').textContent = v ? 'searching' : 'idle';
}

function updateStatus() {
  let s;
  if (game.in_checkmate()) s = (game.turn() === 'w' ? 'Black' : 'White') + ' wins (mate)';
  else if (game.in_stalemate()) s = 'Draw (stalemate)';
  else if (game.in_threefold_repetition()) s = 'Draw (3-fold)';
  else if (game.insufficient_material()) s = 'Draw (material)';
  else if (game.in_draw()) s = 'Draw (50-move)';
  else s = (game.turn() === 'w' ? 'White' : 'Black') + ' to move' + (game.in_check() ? ' (check)' : '');
  $status.textContent = s;
  $fen.value = game.fen();
  highlightCheck();
}

function highlightCheck() {
  $('#board .square-55d63').removeClass('check-highlight');
  if (!game.in_check() && !game.in_checkmate()) return;
  const turn = game.turn();
  const rows = game.board();
  for (let r = 0; r < 8; ++r) {
    for (let f = 0; f < 8; ++f) {
      const p = rows[r][f];
      if (p && p.type === 'k' && p.color === turn) {
        const sq = 'abcdefgh'[f] + (8 - r);
        $('#board .square-' + sq).addClass('check-highlight');
        return;
      }
    }
  }
}

function highlightMove(from, to) {
  $('#board .square-55d63').removeClass('highlight-last');
  if (from) $('#board .square-' + from).addClass('highlight-last');
  if (to) $('#board .square-' + to).addClass('highlight-last');
  lastFromSq = from; lastToSq = to;
}

function squareCenter(sq) {
  const file = 'abcdefgh'.indexOf(sq[0]);
  const rank = parseInt(sq[1], 10) - 1;
  const S = 80; // 640 / 8
  const flipped = board && board.orientation() === 'black';
  const col = flipped ? 7 - file : file;
  const row = flipped ? rank : 7 - rank;
  return { x: col * S + S / 2, y: row * S + S / 2 };
}

function isLegalHere(uciMove) {
  if (!uciMove || uciMove.length < 4) return false;
  const from = uciMove.slice(0, 2), to = uciMove.slice(2, 4);
  return game.moves({ square: from, verbose: true }).some(m => m.to === to);
}

function drawEngineArrow(uciMove) {
  const svg = document.getElementById('arrow-layer');
  // clear existing arrow/label but keep <defs>
  svg.querySelectorAll('line, rect, text').forEach(n => n.remove());
  if (!uciMove || uciMove.length < 4) return;
  if (!isLegalHere(uciMove)) return;
  const a = squareCenter(uciMove.slice(0, 2));
  const b = squareCenter(uciMove.slice(2, 4));
  // shorten so arrowhead doesn't swallow whole square
  const dx = b.x - a.x, dy = b.y - a.y;
  const len = Math.hypot(dx, dy) || 1;
  const shrink = 16;
  const bx = b.x - (dx / len) * shrink;
  const by = b.y - (dy / len) * shrink;
  const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
  line.setAttribute('x1', a.x); line.setAttribute('y1', a.y);
  line.setAttribute('x2', bx);  line.setAttribute('y2', by);
  line.setAttribute('stroke', '#42a5f5');
  line.setAttribute('stroke-width', '10');
  line.setAttribute('stroke-linecap', 'round');
  line.setAttribute('stroke-opacity', '0.75');
  line.setAttribute('marker-end', 'url(#arrowhead)');
  svg.appendChild(line);

  if (currentScoreText) {
    const NS = 'http://www.w3.org/2000/svg';
    const mx = (a.x + b.x) / 2, my = (a.y + b.y) / 2;
    const rect = document.createElementNS(NS, 'rect');
    const text = document.createElementNS(NS, 'text');
    text.setAttribute('x', mx); text.setAttribute('y', my);
    text.setAttribute('text-anchor', 'middle');
    text.setAttribute('dominant-baseline', 'central');
    text.setAttribute('font-family', 'Consolas, monospace');
    text.setAttribute('font-size', '20');
    text.setAttribute('font-weight', '700');
    text.setAttribute('fill', '#fff');
    text.textContent = currentScoreText;
    // rect sized after we know text bbox: use approx width
    const approxW = currentScoreText.length * 12 + 10;
    rect.setAttribute('x', mx - approxW / 2);
    rect.setAttribute('y', my - 14);
    rect.setAttribute('width', approxW);
    rect.setAttribute('height', 28);
    rect.setAttribute('rx', 6);
    rect.setAttribute('fill', '#1565c0');
    rect.setAttribute('fill-opacity', '0.9');
    svg.appendChild(rect);
    svg.appendChild(text);
  }
}

function clearEngineArrow() { currentPvMove = null; currentScoreText = null; drawEngineArrow(null); }

function clearHints() {
  $('#board .square-55d63').removeClass('hint-move hint-capture hint-source');
}

function showHints(square) {
  const moves = game.moves({ square, verbose: true });
  if (!moves.length) return;
  $('#board .square-' + square).addClass('hint-source');
  for (const m of moves) {
    const $sq = $('#board .square-' + m.to);
    $sq.addClass(m.flags.includes('c') || m.flags.includes('e') ? 'hint-capture' : 'hint-move');
  }
}

function onDragStart(source, piece) {
  if (game.game_over()) return false;
  if ((game.turn() === 'w' && piece.startsWith('b')) ||
      (game.turn() === 'b' && piece.startsWith('w'))) return false;
  showHints(source);
}

function onDrop(source, target) {
  clearHints();
  const move = game.move({ from: source, to: target, promotion: 'q' });
  if (!move) return 'snapback';
  // extend or fork the stored line
  loadedGame.moves = loadedGame.moves.slice(0, currentPly);
  loadedGame.moves.push(move);
  currentPly = loadedGame.moves.length;
  renderMovesList();
  clearEngineArrow();
  recordEvalForLastMove();
  highlightMove(source, target);
  updateStatus();
  maybeEngineMove();
}

function onSnapEnd() { board.position(game.fen()); }

board = Chessboard('board', {
  draggable: true,
  position: 'start',
  pieceTheme: 'https://chessboardjs.com/img/chesspieces/wikipedia/{piece}.png',
  onDragStart, onDrop, onSnapEnd
});

updateStatus();
drawEvalChart();
renderMovesList();
updateEvalBar(null);

// ---- UCI parsing ----
function parseInfo(line) {
  const parts = line.split(/\s+/);
  const info = {};
  for (let i = 1; i < parts.length; i++) {
    const tok = parts[i];
    if (tok === 'depth' || tok === 'seldepth' || tok === 'nodes' || tok === 'nps' ||
        tok === 'time' || tok === 'hashfull' || tok === 'multipv') {
      info[tok] = parts[++i];
    } else if (tok === 'tthash' || tok === 'tttotal') {
      info[tok] = parts[++i];
    } else if (tok === 'score') {
      const kind = parts[++i]; const val = parts[++i];
      info.score = kind === 'mate' ? ('#' + val) : (val / 100).toFixed(2);
      info.scoreRaw = { kind, val: Number(val) };
    } else if (tok === 'pv') {
      info.pv = parts.slice(i + 1).join(' ');
      break;
    }
  }
  return info;
}

function drawEvalChart() {
  const svg = document.getElementById('eval-chart');
  svg.innerHTML = '';
  const W = 640, H = 140, MID = H / 2;
  const NS = 'http://www.w3.org/2000/svg';
  // zero line
  const zero = document.createElementNS(NS, 'line');
  zero.setAttribute('x1', 0); zero.setAttribute('x2', W);
  zero.setAttribute('y1', MID); zero.setAttribute('y2', MID);
  zero.setAttribute('stroke', '#444'); zero.setAttribute('stroke-dasharray', '4,4');
  svg.appendChild(zero);

  if (evalHistory.length === 0) return;
  const CAP = 1000; // clamp eval display to ±10 pawns
  const clamp = v => Math.max(-CAP, Math.min(CAP, v));
  const plyMax = Math.max(evalHistory[evalHistory.length - 1].ply, 20);
  const xOf = p => (p / plyMax) * W;
  const yOf = cp => MID - (clamp(cp) / CAP) * (MID - 6);

  // area fill (white side above zero)
  let d = `M 0 ${MID}`;
  for (const pt of evalHistory) d += ` L ${xOf(pt.ply)} ${yOf(pt.cp)}`;
  d += ` L ${xOf(evalHistory[evalHistory.length - 1].ply)} ${MID} Z`;
  const area = document.createElementNS(NS, 'path');
  area.setAttribute('d', d);
  area.setAttribute('fill', '#42a5f5');
  area.setAttribute('fill-opacity', '0.18');
  svg.appendChild(area);

  // line
  let dl = '';
  evalHistory.forEach((pt, i) => {
    dl += (i === 0 ? 'M ' : ' L ') + xOf(pt.ply) + ' ' + yOf(pt.cp);
  });
  const line = document.createElementNS(NS, 'path');
  line.setAttribute('d', dl);
  line.setAttribute('fill', 'none');
  line.setAttribute('stroke', '#42a5f5');
  line.setAttribute('stroke-width', '2');
  svg.appendChild(line);

  // points
  for (const pt of evalHistory) {
    const c = document.createElementNS(NS, 'circle');
    c.setAttribute('cx', xOf(pt.ply));
    c.setAttribute('cy', yOf(pt.cp));
    c.setAttribute('r', 2.5);
    c.setAttribute('fill', '#e3f2fd');
    svg.appendChild(c);
  }
}

function resetEvalHistory() {
  lastCp = null;
  evalHistory = [];
  drawEvalChart();
  updateEvalBar(null);
}

function recordEvalForLastMove() {
  if (lastCp === null) return;
  evalHistory.push({ ply: game.history().length, cp: lastCp });
  drawEvalChart();
}

function applyInfo(info) {
  for (const k of ids) if (info[k] !== undefined) $i[k].textContent = info[k];
  if (info.tthash !== undefined) {
    const used = Number(info.tthash);
    const total = info.tttotal !== undefined ? Number(info.tttotal) : null;
    $i.tthash.textContent = total
      ? `${used.toLocaleString()} / ${total.toLocaleString()}`
      : used.toLocaleString();
  }
  if (info.nodes !== undefined && searchStartMs) {
    const elapsed = Math.max(1, Date.now() - searchStartMs);
    if (info.time === undefined) $i.time.textContent = elapsed;
    if (info.nps === undefined) {
      const nps = Math.round(Number(info.nodes) * 1000 / elapsed);
      $i.nps.textContent = nps.toLocaleString();
    }
  }
  if (info.scoreRaw) {
    let cp;
    if (info.scoreRaw.kind === 'mate') cp = info.scoreRaw.val >= 0 ? 10000 : -10000;
    else cp = info.scoreRaw.val;
    if (game.turn() === 'b') cp = -cp;
    lastCp = cp;
    updateEvalBar(cp);
  }
  if (info.score !== undefined) {
    let disp = info.score;
    // Flip for black-to-move so eval is always from white's POV
    if (info.scoreRaw && game.turn() === 'b' && info.scoreRaw.kind === 'cp') {
      disp = (-info.scoreRaw.val / 100).toFixed(2);
    } else if (info.scoreRaw && game.turn() === 'b' && info.scoreRaw.kind === 'mate') {
      disp = '#' + (-info.scoreRaw.val);
    }
    $engineEval.textContent = disp;
    $i.score.textContent = info.score;
    if (disp !== currentScoreText) {
      currentScoreText = disp;
      if (currentPvMove) drawEngineArrow(currentPvMove);
    }
  }
  if (info.pv) {
    $pvLine.textContent = 'pv: ' + info.pv;
    const first = info.pv.split(/\s+/)[0];
    if (first && first !== currentPvMove) {
      currentPvMove = first;
      drawEngineArrow(first);
    }
  }
}

function handleEngineLine(line) {
  appendLog('< ' + line);
  if (line.startsWith('id name ')) $engineName.textContent = line.slice(8);
  else if (line === 'uciok') { engineReady = true; sendCmd('isready'); }
  else if (line === 'readyok') { /* noop */ }
  else if (line.startsWith('info ') && line.includes(' depth ')) applyInfo(parseInfo(line));
  else if (line.startsWith('bestmove ')) {
    setThinking(false);
    const uci = line.split(/\s+/)[1];
    lastBestmove = uci;
    // keep the arrow visible, anchored to the engine's final choice
    if (uci && uci !== '(none)' && uci !== '0000') {
      currentPvMove = uci;
      drawEngineArrow(uci);
    }
    if (pgnAnalysis && pgnAnalysis.resolveBestmove) {
      const r = pgnAnalysis.resolveBestmove;
      pgnAnalysis.resolveBestmove = null;
      r({ uci, cp: lastCp });
      return;
    }
    if (selfPlay) {
      const played = playBestmove(uci);
      const reason = gameEndReason();
      if (!played || reason) {
        stopSelfPlay();
        if (reason) appendLog('[selfplay stopped: ' + reason + ']');
        else appendLog('[selfplay stopped: engine returned no move]');
      } else {
        clearEngineArrow();
        sendCmd(positionCmd());
        setThinking(true);
        sendCmd(buildGoCmd());
        return;
      }
    } else if (document.getElementById('auto').checked) {
      playBestmove(uci);
    }
    if (pendingGo) {
      const cmd = pendingGo; pendingGo = null;
      clearEngineArrow();
      sendCmd(positionCmd());
      setThinking(true);
      searchStartMs = Date.now();
      sendCmd(cmd);
    } else if (pendingRestart) {
      pendingRestart = false;
      startInfiniteAnalysis();
    }
  }
}

function playBestmove(uci) {
  if (!uci || uci === '(none)' || uci === '0000') return false;
  const from = uci.slice(0, 2), to = uci.slice(2, 4), promo = uci.slice(4, 5) || undefined;
  const mv = game.move({ from, to, promotion: promo || 'q' });
  if (!mv) return false;
  loadedGame.moves = loadedGame.moves.slice(0, currentPly);
  loadedGame.moves.push(mv);
  currentPly = loadedGame.moves.length;
  renderMovesList();
  board.position(game.fen());
  clearEngineArrow();
  recordEvalForLastMove();
  highlightMove(from, to);
  updateStatus();
  restartAnalysisIfOn();
  return true;
}

function renderMovesList() {
  const el = document.getElementById('moves-list');
  if (!loadedGame.moves.length) {
    el.innerHTML = '<div class="moves-empty">No game loaded</div>';
    return;
  }
  const parts = [];
  for (let i = 0; i < loadedGame.moves.length; i++) {
    const m = loadedGame.moves[i];
    if (m.color === 'w') parts.push(`<span class="move-num">${Math.floor(i / 2) + 1}.</span>`);
    const active = i + 1 === currentPly ? ' move-active' : '';
    parts.push(`<span class="move${active}" data-ply="${i + 1}">${m.san}</span>`);
  }
  el.innerHTML = parts.join(' ');
  el.querySelectorAll('.move').forEach(s => {
    s.onclick = () => goToPly(+s.dataset.ply);
  });
  const act = el.querySelector('.move-active');
  if (act) act.scrollIntoView({ block: 'nearest' });
}

function goToPly(n) {
  n = Math.max(0, Math.min(loadedGame.moves.length, n));
  if (loadedGame.startFen) game.load(loadedGame.startFen);
  else game.reset();
  for (let i = 0; i < n; i++) game.move(loadedGame.moves[i]);
  currentPly = n;
  board.position(game.fen());
  if (n > 0) {
    const last = loadedGame.moves[n - 1];
    highlightMove(last.from, last.to);
  } else {
    highlightMove(null, null);
  }
  clearEngineArrow();
  updateStatus();
  renderMovesList();
  restartAnalysisIfOn();
}

function importPgn(pgnText) {
  const probe = new Chess();
  if (!probe.load_pgn(pgnText, { sloppy: true })) {
    appendLog('[pgn] could not parse file');
    return;
  }
  const moves = probe.history({ verbose: true });
  if (!moves.length) { appendLog('[pgn] no moves found'); return; }
  // force off conflicting modes
  document.getElementById('auto').checked = false;
  document.getElementById('analyze').checked = false;
  if (selfPlay) { stopSelfPlay(); if (thinking) sendCmd('stop'); }

  loadedGame = {
    moves,
    headers: probe.header(),
    startFen: probe.header().FEN || null
  };
  startFen = loadedGame.startFen;
  currentPly = 0;
  if (loadedGame.startFen) game.load(loadedGame.startFen);
  else game.reset();
  board.position(game.fen());
  highlightMove(null, null);
  clearEngineArrow();
  resetEvalHistory();
  updateStatus();
  renderMovesList();
  const h = loadedGame.headers;
  appendLog(`[pgn] imported ${moves.length} moves` +
    (h.White || h.Black ? ` (${h.White || '?'} vs ${h.Black || '?'})` : ''));
}

function runSearch(goCmd) {
  return new Promise((resolve) => {
    pgnAnalysis.resolveBestmove = resolve;
    lastCp = null;
    clearEngineArrow();
    sendCmd(positionCmd());
    setThinking(true);
    searchStartMs = Date.now();
    sendCmd(goCmd);
  });
}

// Lichess-style accuracy
function winPct(cp) {
  const c = Math.max(-1000, Math.min(1000, cp));
  return 50 + 50 * (2 / (1 + Math.exp(-0.00368208 * c)) - 1);
}
function accuracyFromWinLoss(wl) {
  const a = 103.1668 * Math.exp(-0.04354 * wl) - 3.1669;
  return Math.max(0, Math.min(100, a));
}

async function analyzeLoadedGame() {
  if (!engineLoaded) { appendLog('[analyze] load an engine first'); return; }
  if (!loadedGame.moves.length) { appendLog('[analyze] no game loaded'); return; }
  if (pgnAnalysis && pgnAnalysis.active) { appendLog('[analyze] already analyzing'); return; }

  const moves = loadedGame.moves;

  // force off conflicting modes
  document.getElementById('auto').checked = false;
  document.getElementById('analyze').checked = false;
  if (selfPlay) stopSelfPlay();

  // rewind to the start
  goToPly(0);
  resetEvalHistory();

  const panel = document.getElementById('accuracy-panel');
  const $prog = document.getElementById('accuracy-progress');
  const $sum = document.getElementById('accuracy-summary');
  panel.style.display = 'block';
  $sum.textContent = '';
  $prog.textContent = 'Parsing… ' + moves.length + ' moves';

  const goCmd = buildGoCmd();
  pgnAnalysis = { active: true, cancel: false, resolveBestmove: null };

  // drain any in-flight search
  if (thinking) {
    await new Promise((res) => { pgnAnalysis.resolveBestmove = res; sendCmd('stop'); });
  }

  const report = [];
  // analyze position before move 0
  let before = await runSearch(goCmd);
  if (pgnAnalysis.cancel) { return finishPgn(null); }

  for (let i = 0; i < moves.length; i++) {
    const mv = moves[i];
    const actualUci = mv.from + mv.to + (mv.promotion || '');
    const side = mv.color; // 'w' or 'b'

    // advance through the stored line so the moves-list highlight follows along
    goToPly(i + 1);
    // record the eval going into this move in the chart (snapshot of cpBefore)
    if (before.cp !== null) { evalHistory.push({ ply: i + 1, cp: before.cp }); drawEvalChart(); }

    $prog.textContent = `Move ${i + 1} / ${moves.length}  (${side === 'w' ? 'White' : 'Black'}: ${mv.san})`;

    // analyze the resulting position to get cpAfter
    let after;
    if (game.game_over()) {
      after = { uci: null, cp: before.cp };
    } else {
      after = await runSearch(goCmd);
      if (pgnAnalysis.cancel) return finishPgn(null);
    }

    const cpBefore = before.cp, cpAfter = after.cp;
    let accuracy = null;
    if (cpBefore !== null && cpAfter !== null) {
      const movingWinBefore = side === 'w' ? winPct(cpBefore) : 100 - winPct(cpBefore);
      const movingWinAfter  = side === 'w' ? winPct(cpAfter)  : 100 - winPct(cpAfter);
      const wl = Math.max(0, movingWinBefore - movingWinAfter);
      accuracy = accuracyFromWinLoss(wl);
    }
    const match = actualUci === before.uci;
    const cpLoss = (cpBefore !== null && cpAfter !== null)
      ? (side === 'w' ? cpBefore - cpAfter : cpAfter - cpBefore) : null;

    report.push({ ply: i + 1, side, san: mv.san, actualUci, engineUci: before.uci,
                  cpBefore, cpAfter, cpLoss, accuracy, match });

    appendLog(`[pgn] ${i + 1}. ${mv.san.padEnd(8)} engine=${before.uci || '—'} ` +
              `cp ${cpBefore ?? '—'}→${cpAfter ?? '—'} ` +
              `loss=${cpLoss ?? '—'} acc=${accuracy !== null ? accuracy.toFixed(1) : '—'}` +
              (match ? ' ✓' : ''));

    before = after;
  }

  finishPgn(report);
}

function finishPgn(report) {
  const panel = document.getElementById('accuracy-panel');
  const $prog = document.getElementById('accuracy-progress');
  const $sum = document.getElementById('accuracy-summary');
  if (!report) {
    $prog.textContent = 'Cancelled.';
    pgnAnalysis = null;
    return;
  }
  const whiteMoves = report.filter(r => r.side === 'w' && r.accuracy !== null);
  const blackMoves = report.filter(r => r.side === 'b' && r.accuracy !== null);
  const avg = arr => arr.length ? arr.reduce((a, r) => a + r.accuracy, 0) / arr.length : NaN;
  const bucket = (r) => {
    if (r.cpLoss === null) return 'unknown';
    if (r.cpLoss < 30) return 'good';
    if (r.cpLoss < 90) return 'inacc';
    if (r.cpLoss < 200) return 'mistake';
    return 'blunder';
  };
  const count = (side, kind) => report.filter(r => r.side === side && bucket(r) === kind).length;
  const wAcc = avg(whiteMoves), bAcc = avg(blackMoves);
  const wMatch = report.filter(r => r.side === 'w' && r.match).length;
  const bMatch = report.filter(r => r.side === 'b' && r.match).length;
  const wTotal = report.filter(r => r.side === 'w').length;
  const bTotal = report.filter(r => r.side === 'b').length;

  $prog.textContent = 'Done — ' + report.length + ' moves analyzed';
  $sum.textContent =
    `White accuracy: ${isNaN(wAcc) ? '—' : wAcc.toFixed(1)}%    Black accuracy: ${isNaN(bAcc) ? '—' : bAcc.toFixed(1)}%\n` +
    `Top-1 match:    W ${wMatch}/${wTotal}                    B ${bMatch}/${bTotal}\n` +
    `Inaccuracies:   W ${count('w','inacc')}  B ${count('b','inacc')}\n` +
    `Mistakes:       W ${count('w','mistake')}  B ${count('b','mistake')}\n` +
    `Blunders:       W ${count('w','blunder')}  B ${count('b','blunder')}`;
  panel.style.display = 'block';
  pgnAnalysis = null;
}

function stopSelfPlay() {
  selfPlay = false;
  document.getElementById('btn-selfplay').classList.remove('active');
}

function gameEndReason() {
  if (!game.game_over()) return null;
  if (game.in_checkmate()) return 'checkmate';
  if (game.in_stalemate()) return 'stalemate';
  if (game.insufficient_material()) return 'insufficient material';
  if (game.in_threefold_repetition()) return '3-fold repetition';
  if (game.in_draw()) return '50-move rule';
  return 'game over';
}

// ---- Sending ----
function sendCmd(cmd) {
  appendLog('> ' + cmd);
  window.viking.send(cmd);
}

function positionCmd() {
  const hist = game.history({ verbose: true })
    .map(m => m.from + m.to + (m.promotion || ''))
    .join(' ');
  const base = startFen ? ('position fen ' + startFen) : 'position startpos';
  return base + (hist ? ' moves ' + hist : '');
}

function buildGoCmd() {
  const mt = document.getElementById('movetime').value;
  const d = document.getElementById('depth').value;
  let go = 'go';
  if (d) go += ' depth ' + d;
  else if (mt) go += ' movetime ' + mt;
  return go;
}

function goClicked() {
  if (!engineLoaded) return;
  if (thinking) {
    pendingGo = buildGoCmd();
    pendingRestart = false;
    sendCmd('stop');
    return;
  }
  clearEngineArrow();
  sendCmd(positionCmd());
  setThinking(true);
  searchStartMs = Date.now();
  sendCmd(buildGoCmd());
}

function stopClicked() {
  if (pgnAnalysis && pgnAnalysis.active) {
    pgnAnalysis.cancel = true;
    pgnAnalysis.active = false;
    if (thinking) sendCmd('stop');
    return;
  }
  if (thinking) sendCmd('stop');
}

function startInfiniteAnalysis() {
  if (!engineLoaded || game.game_over()) return;
  clearEngineArrow();
  sendCmd(positionCmd());
  setThinking(true);
  searchStartMs = Date.now();
  sendCmd('go infinite');
}

function restartAnalysisIfOn() {
  if (!document.getElementById('analyze').checked) return;
  if (!engineLoaded) return;
  if (thinking) {
    pendingRestart = true;
    sendCmd('stop');
  } else {
    startInfiniteAnalysis();
  }
}

function maybeEngineMove() {
  if (document.getElementById('auto').checked && engineLoaded && !game.game_over()) {
    goClicked();
  } else {
    restartAnalysisIfOn();
  }
}

// ---- Wire up ----
window.viking.onLine(handleEngineLine);
window.viking.onExit((code) => {
  appendLog('[engine exited code=' + code + ']');
  engineLoaded = false; engineReady = false;
  $engineName.textContent = 'No engine loaded';
});

function sendEngineOptions() {
  const t = parseInt(document.getElementById('threads').value, 10);
  const h = parseInt(document.getElementById('hash').value, 10);
  if (Number.isFinite(t) && t >= 1) sendCmd('setoption name Threads value ' + t);
  if (Number.isFinite(h) && h >= 1) sendCmd('setoption name Hash value ' + h);
}

document.getElementById('btn-load').onclick = async () => {
  const p = await window.viking.pickEngine();
  if (!p) return;
  const r = await window.viking.loadEngine(p);
  if (!r.ok) { appendLog('[load failed] ' + r.error); return; }
  engineLoaded = true;
  appendLog('[loaded] ' + p);
  sendCmd('uci');
  sendEngineOptions();
};

document.getElementById('threads').addEventListener('change', () => {
  if (!engineLoaded) return;
  const t = parseInt(document.getElementById('threads').value, 10);
  if (Number.isFinite(t) && t >= 1) sendCmd('setoption name Threads value ' + t);
});
document.getElementById('hash').addEventListener('change', () => {
  if (!engineLoaded) return;
  const h = parseInt(document.getElementById('hash').value, 10);
  if (Number.isFinite(h) && h >= 1) sendCmd('setoption name Hash value ' + h);
});

document.getElementById('btn-new').onclick = () => {
  if (selfPlay) {
    stopSelfPlay();
    if (thinking) sendCmd('stop');
  }
  game.reset(); startFen = null; board.position('start'); highlightMove(null, null); clearEngineArrow();
  loadedGame = { moves: [], startFen: null, headers: {} };
  currentPly = 0;
  renderMovesList();
  resetEvalHistory();
  $engineEval.textContent = '—'; $pvLine.textContent = 'pv: —';
  for (const k of ids) $i[k].textContent = '—';
  if (engineLoaded) sendCmd('ucinewgame');
  updateStatus();
};

document.getElementById('btn-undo').onclick = () => {
  if (game.undo()) {
    if (loadedGame.moves.length >= currentPly && currentPly > 0) {
      loadedGame.moves = loadedGame.moves.slice(0, currentPly - 1);
      currentPly--;
    }
    renderMovesList();
    evalHistory.pop(); drawEvalChart();
    board.position(game.fen()); highlightMove(null, null); clearEngineArrow(); updateStatus(); restartAnalysisIfOn();
  }
};
document.getElementById('btn-flip').onclick = () => {
  board.flip();
  document.getElementById('eval-bar').dataset.orient = board.orientation() === 'black' ? 'black' : 'white';
  updateEvalBar(lastCp);
  if (currentPvMove) drawEngineArrow(currentPvMove);
};

document.getElementById('btn-pgn').onclick = () => document.getElementById('pgn-file').click();

document.getElementById('btn-pgn-export').onclick = () => {
  if (!game.history().length && !startFen) { appendLog('[pgn] nothing to export'); return; }
  const now = new Date();
  const dateStr = now.getFullYear() + '.' +
    String(now.getMonth() + 1).padStart(2, '0') + '.' +
    String(now.getDate()).padStart(2, '0');
  const engineName = document.getElementById('engine-name').textContent;
  const headers = {
    Event: 'Viking GUI game',
    Site: 'Viking GUI',
    Date: dateStr,
    White: engineName !== 'No engine loaded' ? engineName : 'White',
    Black: engineName !== 'No engine loaded' ? engineName : 'Black',
    Result: game.in_checkmate() ? (game.turn() === 'w' ? '0-1' : '1-0')
          : game.game_over() ? '1/2-1/2' : '*'
  };
  if (startFen) { headers.SetUp = '1'; headers.FEN = startFen; }
  game.header(...Object.entries(headers).flat());
  const pgn = game.pgn({ max_width: 80, newline_char: '\n' });
  const blob = new Blob([pgn + '\n'], { type: 'application/x-chess-pgn' });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = 'viking-' + dateStr.replace(/\./g, '') + '-' +
    String(now.getHours()).padStart(2, '0') +
    String(now.getMinutes()).padStart(2, '0') + '.pgn';
  document.body.appendChild(a); a.click(); a.remove();
  URL.revokeObjectURL(url);
  appendLog('[pgn] exported ' + a.download);
};
document.getElementById('pgn-file').addEventListener('change', async (e) => {
  const file = e.target.files[0];
  if (!file) return;
  e.target.value = '';
  const text = await file.text();
  importPgn(text);
});

document.getElementById('btn-analyze').onclick = () => analyzeLoadedGame();

document.getElementById('btn-first').onclick = () => goToPly(0);
document.getElementById('btn-prev').onclick  = () => goToPly(currentPly - 1);
document.getElementById('btn-next').onclick  = () => goToPly(currentPly + 1);
document.getElementById('btn-last').onclick  = () => goToPly(loadedGame.moves.length);

document.addEventListener('keydown', (e) => {
  if (['INPUT', 'TEXTAREA'].includes(document.activeElement?.tagName)) return;
  if (!loadedGame.moves.length) return;
  if (e.key === 'ArrowLeft')  { goToPly(currentPly - 1); e.preventDefault(); }
  else if (e.key === 'ArrowRight') { goToPly(currentPly + 1); e.preventDefault(); }
  else if (e.key === 'Home')       { goToPly(0); e.preventDefault(); }
  else if (e.key === 'End')        { goToPly(loadedGame.moves.length); e.preventDefault(); }
});
document.getElementById('btn-go').onclick = goClicked;
document.getElementById('btn-stop').onclick = stopClicked;

document.getElementById('btn-selfplay').onclick = () => {
  const btn = document.getElementById('btn-selfplay');
  if (selfPlay) {
    selfPlay = false;
    btn.classList.remove('active');
    if (thinking) sendCmd('stop');
    return;
  }
  if (!engineLoaded || game.game_over()) return;
  // self-play drives the engine itself; force the other modes off
  document.getElementById('auto').checked = false;
  if (document.getElementById('analyze').checked) {
    document.getElementById('analyze').checked = false;
  }
  pendingRestart = false;
  selfPlay = true;
  btn.classList.add('active');
  if (thinking) {
    pendingGo = buildGoCmd();
    sendCmd('stop');
  } else {
    clearEngineArrow();
    sendCmd(positionCmd());
    setThinking(true);
    sendCmd(buildGoCmd());
  }
};

function tryLoadFen(f) {
  f = (f || '').trim();
  if (!f) return false;
  if (!game.load(f)) { appendLog('[bad FEN] ' + f); return false; }
  if (selfPlay) { stopSelfPlay(); if (thinking) sendCmd('stop'); }
  startFen = f;
  loadedGame = { moves: [], startFen: f, headers: {} };
  currentPly = 0;
  renderMovesList();
  board.position(game.fen());
  highlightMove(null, null);
  clearEngineArrow();
  resetEvalHistory();
  updateStatus();
  restartAnalysisIfOn();
  return true;
}

$fen.addEventListener('keydown', (e) => {
  if (e.key !== 'Enter') return;
  tryLoadFen($fen.value);
});

$fen.addEventListener('paste', (e) => {
  const text = (e.clipboardData || window.clipboardData).getData('text');
  if (!text) return;
  e.preventDefault();
  $fen.value = text.trim();
  tryLoadFen($fen.value);
});

document.getElementById('analyze').addEventListener('change', (e) => {
  if (e.target.checked) {
    if (thinking) { pendingRestart = true; sendCmd('stop'); }
    else startInfiniteAnalysis();
  } else {
    if (thinking) sendCmd('stop');
    pendingRestart = false;
  }
});

const $sendInput = document.getElementById('send-input');
$sendInput.addEventListener('keydown', (e) => {
  if (e.key !== 'Enter') return;
  const c = $sendInput.value.trim();
  if (!c) return;
  sendCmd(c); $sendInput.value = '';
});

// ---- Clear log ----
document.getElementById('btn-clear-log').onclick = () => {
  $log.textContent = '';
};

// ---- Board auto-fit ----
function layoutBoard() {
  const row = document.getElementById('board-row');
  const evalBar = document.getElementById('eval-bar');
  const wrap = document.getElementById('board-wrap');
  if (!wrap || !row) return;
  const gap = 10;
  const availW = Math.max(0, row.clientWidth - evalBar.offsetWidth - gap);
  const availH = row.clientHeight;
  const size = Math.max(120, Math.floor(Math.min(availW, availH)));
  wrap.style.width = size + 'px';
  wrap.style.height = size + 'px';
  evalBar.style.height = size + 'px';
  if (board && typeof board.resize === 'function') board.resize();
}

window.addEventListener('resize', layoutBoard);
new ResizeObserver(() => layoutBoard()).observe(document.getElementById('left'));
requestAnimationFrame(layoutBoard);

// ---- Resizable splitters (persist in localStorage) ----
(function setupSplitters() {
  const LS_LEFT_W = 'viking.leftWidth';
  const LS_LOG_H = 'viking.logHeight';
  const leftEl = document.getElementById('left');
  const logEl = document.getElementById('log');
  const splitterV = document.getElementById('splitter-v');
  const splitterH = document.getElementById('splitter-h');

  // Restore saved sizes.
  const savedLeft = parseInt(localStorage.getItem(LS_LEFT_W), 10);
  if (Number.isFinite(savedLeft) && savedLeft >= 300) {
    leftEl.style.flex = '0 0 ' + savedLeft + 'px';
  }
  const savedLogH = parseInt(localStorage.getItem(LS_LOG_H), 10);
  if (Number.isFinite(savedLogH) && savedLogH >= 60) {
    logEl.style.flex = '0 0 auto';
    logEl.style.height = savedLogH + 'px';
  }

  // Vertical splitter: drag to resize the left (board) column.
  splitterV.addEventListener('mousedown', (e) => {
    e.preventDefault();
    const startX = e.clientX;
    const startW = leftEl.getBoundingClientRect().width;
    splitterV.classList.add('dragging');
    document.body.classList.add('resizing');
    const onMove = (ev) => {
      const w = Math.max(300, Math.min(window.innerWidth - 250, startW + (ev.clientX - startX)));
      leftEl.style.flex = '0 0 ' + w + 'px';
    };
    const onUp = () => {
      document.removeEventListener('mousemove', onMove);
      document.removeEventListener('mouseup', onUp);
      splitterV.classList.remove('dragging');
      document.body.classList.remove('resizing');
      localStorage.setItem(LS_LEFT_W, leftEl.getBoundingClientRect().width | 0);
    };
    document.addEventListener('mousemove', onMove);
    document.addEventListener('mouseup', onUp);
  });

  // Horizontal splitter: drag to resize the engine log (grows when dragged up).
  splitterH.addEventListener('mousedown', (e) => {
    e.preventDefault();
    const startY = e.clientY;
    const startH = logEl.getBoundingClientRect().height;
    logEl.style.flex = '0 0 auto';
    splitterH.classList.add('dragging');
    document.body.classList.add('resizing-v');
    const onMove = (ev) => {
      const h = Math.max(60, Math.min(window.innerHeight - 200, startH + (startY - ev.clientY)));
      logEl.style.height = h + 'px';
    };
    const onUp = () => {
      document.removeEventListener('mousemove', onMove);
      document.removeEventListener('mouseup', onUp);
      splitterH.classList.remove('dragging');
      document.body.classList.remove('resizing-v');
      localStorage.setItem(LS_LOG_H, parseInt(logEl.style.height, 10));
    };
    document.addEventListener('mousemove', onMove);
    document.addEventListener('mouseup', onUp);
  });
})();
