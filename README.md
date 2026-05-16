<div align="center">
 <img width="1200" height="340" alt="animated honeybee democracy banner" src="https://github.com/user-attachments/assets/143cb360-0499-4b4a-acc9-d2a3535b129e" />
</div>

A C++ / ImGui simulation of Thomas D. Seeley's *Honeybee Democracy*: scout bees waggle-dance to advertise candidate nest sites, and a genetic algorithm searches for the strategy that lets a swarm reach the best consensus.

## The honeybee democracy

Every spring a honeybee colony grows too big for its hive and roughly 10,000 workers leave with the old queen to find a new home. The swarm clusters on a nearby branch while a few hundred **scout bees** fan out, inspect tree cavities, and come back to report. Each scout that returns dances on the swarm's surface to advertise the site she found.

<div align="center">
 <img height="250" alt="honeybee waggle dance" src="https://github.com/user-attachments/assets/e174c784-718b-4406-bdad-a12ef59851b8" />
</div>

The dance is a tight figure-eight whose **angle from vertical** encodes the direction to the site relative to the sun, whose **duration** encodes the distance, and whose **vigor and number of repeats** encode the cavity's quality — a great site gets a long, enthusiastic dance. Other scouts watch, fly out to evaluate independently, and either return enthusiastic or not. Crucially, every dancer's enthusiasm **decays over time**, which keeps an early scout's stale assessment from dominating. Once enough scouts coincide at one site, the swarm commits and takes off together.

<div align="center">
 <img height="200" alt="honeybee democracy book cover" src="https://github.com/user-attachments/assets/96486e4e-c6fb-4b64-8f72-ed7904427f08" />
</div>

This decentralized recruit–evaluate–decay–commit loop finds the best of dozens of candidate cavities almost every time. Tom Seeley spent decades reverse-engineering it and lays the full story out in *Honeybee Democracy* (2010), including its parallels to good human decision-making.

## What the simulation models

The simulation is a simplified, headless version of that scout-and-dance loop, parallelised across many small swarms so a genetic algorithm can search for the best strategy. Each `Hive` holds ~100 scout bees and lives at a fixed location in a shared 2D world. Around them, ~20 candidate nest boxes are scattered in `[-1, 1]²`; their qualities are sampled from `u⁴` so most boxes are poor and one rare box is ideal — the swarm has to find that needle.

Each scout runs a five-state machine — `Rest → Search → BackToHome → Dance → Rest` or `Rest → Follow → Find → BackToHome → Dance → Rest` — controlled by four real-valued **strategy genes** per swarm:

| Gene | What it controls |
|------|------------------|
| `random_chance` | Probability per step that a resting bee leaves to scout a new site at random |
| `follow_chance` | Probability per step that a resting bee follows a dancing nestmate |
| `linear_decay` | How fast a dancing bee's enthusiasm bleeds away |
| `dance_force_exponent` | How strongly site quality maps to dance vigor |

Several swarms with different gene values share the world and run the same exploration task for thousands of steps; a swarm's fitness is the maximum `consensus_fraction × site_goodness` it ever reaches during a run. Between generations the underperforming swarms are crossed with the best swarm and jittered, and every 15th generation the bottom 10% are wiped and re-seeded with random genes. Over many generations the algorithm converges on swarms whose random / follow / decay / dance-vigor balance reliably steers their bees to the rare good nest.

## Build and run

Dependencies: a C++17 compiler, CMake ≥ 3.14, OpenGL, and X11 / GLFW headers. Everything else (ImGui, ImPlot, GLFW, Eigen) is fetched by CMake.

```bash
./build.sh -x          # release build, then run
./build.sh -d -x       # debug build, then run
```

Developed on Linux; macOS should work, Windows is untested.

## The UI

The Simulation window has three stacked plots driven by a single set of controls — `Start / Pause / Stop / Reset` plus a `Real-time` checkbox that paces the simulation at one step every 25 ms so you can actually watch the bees fly.

**Environment plot** — the 2D world. Hives appear as colored squares, their bees as small dots in the same color, and nest boxes as diamonds coloured by quality on a Jet colormap (blue = poor, red = ideal).

**Fitness plot** — one line per hive showing its best-of-run fitness over generations. The lines climb and converge as the GA finds better strategies.

**Consensus plot** — one mini polar dance per hive, side by side. For each nest box that some of a hive's bees are currently dancing for, a rectangle radiates from the hive, its angle pointing toward the box, its length matching the distance to the box, and its **width proportional to the fraction of bees dancing for it**. Watch a hive narrow from many short rectangles to one long bold one as consensus forms.
