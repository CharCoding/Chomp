'use strict';
const multipliers = new Float64Array(14),
  divisors = new Float64Array(14),
  dict = Uint16Array.of(0x336, 0x347, 0x355, 0x448, 0x477, /*0x1125,*/ 0x2226, 0x2237, 0x2248, 0x2255, 0x2335, 0x2357, 0x2368, 0x2447, 0x2458, 0x2588, 0x3338, 0x3377, 0x3457, 0x4557),
  ppos = [],
  wpos = new Set(),
  binSearch = (x, min = 0, max = ppos.length - 1) => {
    while(min <= max) {
      const index = min + max >> 1,
        current = ppos[index];
      if(current < x)
        min = index + 1;
      else if(current > x)
        max = index - 1;
      else
        return index;
    }
    return ~min; // returns -insertion point - 1 if not found
  },
  height = pos => Math.ceil(Math.log2(pos + 1) / 4),
  hm1 = pos => Math.ceil(Math.log2(pos) / 4),
  reflect = pos => { // O(w)
    if((pos & 15) > 13) return 0;
    let h = height(pos), i = 0, newPos = 0;
    do {
      newPos += h * divisors[i];
      pos -= multipliers[h - 1];
      h = height(pos);
      ++i;
    } while(h);
    return newPos;
  },
  sidewaysAdd = int => { // add up all nibbles sideways
    const mask = 0xf0f0f0f;
    int = (int >>> 4 & mask) + (int & mask); // 0 <= int <= 0x1e1e1e1e
    return int * 0x1010101 >>> 24; // just barely under 2**53, don't care about bits > 32 or bits < 24
  },
  squares = pos => sidewaysAdd(pos / 4294967296) + sidewaysAdd(pos),
  losingMove = pos => {
    const w = pos & 15;
    let y = height(pos), max = (pos / divisors[y] & 15) + y;
    for(let i = y; i--;) {
      const row = pos / divisors[i] & 15,
        dist = row + i;
      if(max < dist) {
        max = dist;
        y = i;
      }
      if(row == w)
        break;
    }
    return [max - y - 1, y];
  },
  safe = pos => {
    let w = pos & 15, aux = 0;
    for(let x = 0; x < w; ++x) {
      let h = height(pos) - 1;
      for(let y = +!x; y <= h; ++y) {
        if(binSearch(pos % divisors[y] + aux) >= 0)
          return false;
      }
      aux += multipliers[h];
      pos -= multipliers[h];
    }
    return true;
  },
  heuristicMove = pos => {// O((hw)^2 log(pos))
    const m = [];
    let w = pos & 15, aux = 0;
    for(let x = 0; x < w; ++x) {
      let h = height(pos) - 1;
      for(let y = +!x; y <= h; ++y) {// never check (0, 0)
        let nextPos = pos % divisors[y] + aux;
        if(binSearch(nextPos) >= 0)
          return [x, y];
        if(safe(nextPos))
          m.push([x, y]);
      }
      aux += multipliers[h];
      pos -= multipliers[h];
    }
    if(!m.length) {
      return losingMove(aux);
    }
    return m[Math.random() * m.length | 0];
  },
  isLosing = pos => {
    let index = binSearch(pos);
    if(index >= 0)
      return true;
    const w = pos & 15, h = height(pos);
    if(pos == multipliers[h - 1] * w)
      return false;
    const m = [];
    let aux = 0;
    for(let x = 0; x < w; ++x) {
      let h = height(pos) - 1;
      for(let y = +!x; y <= h; ++y)
        m.push(pos % divisors[y] + aux);
      aux += multipliers[h];
      pos -= multipliers[h];
    }
    const bool = !m.sort((a, b) => squares(a) - squares(b)).some(isLosing);
    if(bool) {
      ppos.splice(~binSearch(aux, ~index), 0, aux);
      pos = reflect(aux);
      if(pos && pos != aux)
        ppos.splice(~binSearch(pos), 0, pos);
    }
    return bool;
  },
  minimax = (pos, depth = 0) => {
    let index = binSearch(pos);
    if(index >= 0)
      return index - ppos.length;
    if(depth == 0)
      return 0;
    /*
    const w = pos & 15, h = height(pos);
    if(pos == multipliers[h - 1] * w)
      return false;
    */
    const m = [], w = pos & 15;
    let aux = 0;
    --depth;
    for(let x = 0; x < w; ++x) {
      let h = height(pos) - 1;
      for(let y = +!x; y <= h; ++y)
        m.push(pos % divisors[y] + aux);
      aux += multipliers[h];
      pos -= multipliers[h];
    }
    m.sort((a, b) => squares(a) - squares(b));
    let max = 0;
    for(let i = 0; i < m.length; ++i) {
      let sub = -minimax(m[i], depth);
      max = Math.max(max, sub);
    }
    return max;
    //const bool = !m.sort((a, b) => squares(a) - squares(b)).some(pos => minimax(pos, depth));
    /*
    if(bool) {
      ppos.splice(~binSearch(aux, ~index), 0, aux);
      pos = reflect(aux);
      if(pos && pos != aux)
        ppos.splice(~binSearch(pos), 0, pos);
    }
    return bool;
    */
  },
  printPos = pos => {
    while(pos > 1) {
      let str = ''.padStart(pos & 15, '#') + ' ' + (pos & 15).toString(16);
      pos /= 16;
      console.log(str);
    }
  },
  findMove = pos => {
    if(pos == 1)
      return [0, 0];
    if(pos < 16)
      return [1, 0];
    const h = height(pos), w = pos & 15;
    if(w == 1)
      return [0, 1];
    if(w == h) {
      if(pos & 0xe0)
        return [1, 1];
      const y = Math.sqrt(Math.random()) * h | 0;
      return [(pos / divisors[y] & 15) - 1, y];
    }
    if(binSearch(pos) >= 0) {
      return losingMove(pos);
    }
    if(squares(pos) > 32) {
      return heuristicMove(pos);
    }
    let aux = 0;
    for(let x = 0; x < w; ++x) {
      let h = height(pos) - 1;
      for(let y = +!x; y <= h; ++y) {// never check (0, 0)
        if(isLosing(pos % divisors[y] + aux))
          return [x, y];
      }
      aux += multipliers[h];
      pos -= multipliers[h];
    }
    return heuristicMove(pos);
  },
  takePos = (pos, x, y) => {
    for(let i = y, h = height(pos); i < h; ++i) {
      const row = pos / divisors[i] & 15;
      if(row > x)
        pos -= (row - x) * divisors[i];
      else
        break;
    }
    return pos;
  },
  populate = () => {
    ppos.length = 0;
    for(let i = Math.min(maxX, maxY); i--;)
      ppos.push(multipliers[i] + i);
    for(let i = maxX - 1; --i;)
      ppos.push(i * 0x11 + 0x12);
    for(let i = maxY - 1; i > 1;)
      ppos.push(multipliers[i] + multipliers[--i]);
    for(let i = maxX - 2; --i;)
      ppos.push(i * 0x11 + 0x213);
    for(let i = maxY - 2; --i;)
      ppos.push(0x11 + multipliers[i] + multipliers[i + 2]);
    /*
    for(let i = 4; i < Math.min(maxX, maxY); i += 2) {
      ppos.push(0x10 + multipliers[i - 1] + i);
      ppos.push(0xf + multipliers[i] + i);
    }
    */
    for(let i = 3, x = 0xf; i < Math.min(maxX, maxY); ++i) // might generate 1 excess ppos
      ppos.push(multipliers[i] + i + (x ^= 0x1e));
    for(let i = 0; i < dict.length; ++i) {
      const pos = dict[i], h = height(pos), w = pos & 15;
      if(h <= maxY && w <= maxX)
        ppos.push(pos);
      if(h <= maxX && w <= maxY)
        ppos.push(reflect(pos));
    }
    ppos.sort((a, b) => a - b);
  },
  validate = () => {
    for(let i = ppos.length - 1; i;)
      if(ppos[i] <= ppos[--i])
        return i;
    return true;
  };
multipliers[0] = divisors[0] = 1;
for(let i = 1; i < 14; ++i) {
  divisors[i] = divisors[i - 1] * 16;
  multipliers[i] = multipliers[i - 1] + divisors[i];
}
let maxX = 15, maxY = 13, currPos = multipliers[maxY - 1] * maxX;
populate();