--
--  Copyright 2008 Chris Desjardins - cjd@chrisd.info
--
--  This program is free software: you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation, either version 3 of the License, or
--  (at your option) any later version.
--
--  This program is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  You should have received a copy of the GNU General Public License
--  along with this program.  If not, see <http://www.gnu.org/licenses/>.
--
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity playerpiano is
    port
    (
        servo_a_pls : buffer std_logic;
        servo_b_pls : buffer std_logic;
        
        stepper_a   : buffer std_logic_vector (3 downto 0) := "0110";
        stepper_b   : buffer std_logic_vector (3 downto 0) := "0110";
        stepper_clk : buffer std_logic;
        
        led_a       : out std_logic;
        led_b       : out std_logic;

        input       : in unsigned (7 downto 0);
        input_clk   : in std_logic;
        debounce_input_clk : buffer std_logic;
        board_clk   : in std_logic;
        
        reset       : in std_logic
    );
end playerpiano;
architecture behav1 of playerpiano is

shared variable stepper_a_pos : signed (15 downto 0) := x"0001";
shared variable stepper_b_pos : signed (15 downto 0) := x"0001";
-- range: 0xcd14 - 0x2710
shared variable servo_a_width : unsigned (15 downto 0) := x"cd14";
shared variable servo_b_width : unsigned (15 downto 0) := x"cd14";

procedure stepit 
(
    signal stepper : inout std_logic_vector (3 downto 0);
    curr_stepper_pos : inout signed (15 downto 0);
    stepper_pos : in signed (15 downto 0)
) is
begin
    if (curr_stepper_pos > stepper_pos) then
        case stepper is
        when x"9" => stepper <= x"5";  -- 0101
        when x"5" => stepper <= x"6";  -- 0110
        when x"6" => stepper <= x"a";  -- 1010
        when others => stepper <= x"9";-- 1001
        end case;
        curr_stepper_pos := curr_stepper_pos - 1;
    elsif (curr_stepper_pos < stepper_pos) then
        case stepper is
        when x"9" => stepper <= x"a";  -- 1010
        when x"5" => stepper <= x"9";  -- 1001
        when x"6" => stepper <= x"5";  -- 0101
        when others => stepper <= x"6";-- 0110
        end case;
        curr_stepper_pos := curr_stepper_pos + 1;
    else
        stepper <= x"0";
    end if;
end procedure;

begin

process (stepper_clk)
variable curr_stepper_a_pos : signed (15 downto 0) := x"0000";
variable curr_stepper_b_pos : signed (15 downto 0) := x"0000";
begin
    if (rising_edge(stepper_clk)) then
        stepit(stepper_a, curr_stepper_a_pos, stepper_a_pos);
        stepit(stepper_b, curr_stepper_b_pos, stepper_b_pos);
    end if;
end process;

process (board_clk)
variable clk_cnt : integer := 0;
begin
    if (rising_edge(board_clk)) then
        clk_cnt := clk_cnt + 1;
        if (clk_cnt = 10000) then
            clk_cnt := 0;
            if (stepper_clk = '1') then
                stepper_clk <=  '0';
            else
                stepper_clk <=  '1';
            end if;
        end if;
    end if;
end process;

process (servo_a_pls)
begin
    
end process;

process (servo_b_pls)
begin
end process;

process (board_clk)
variable clk_cnt : integer := 0;
variable servo_a_clk_cnt : unsigned (15 downto 0) := x"0000";
variable servo_b_clk_cnt : unsigned (15 downto 0) := x"0000";
begin
    if (rising_edge(board_clk)) then
        clk_cnt := clk_cnt + 1;
        if (clk_cnt = 500000) then
            clk_cnt := 0;
            servo_a_pls <=  '1';
            servo_b_pls <=  '1';
            servo_a_clk_cnt := servo_a_width;
            servo_b_clk_cnt := servo_b_width;
        end if;
        
        if (servo_a_pls = '1') then
            servo_a_clk_cnt := servo_a_clk_cnt - 1;
            if (servo_a_clk_cnt = x"0000") then
                servo_a_pls <= '0';
            end if;
        end if;
        
        if (servo_b_pls = '1') then
            servo_b_clk_cnt := servo_b_clk_cnt - 1;
            if (servo_b_clk_cnt = x"0000") then
                servo_b_pls <= '0';
            end if;
        end if;
    end if;
end process;

-- input clock debouncer
process (board_clk)
variable wait_cnt : integer := 0;
variable last : std_logic;
begin
    if (rising_edge(board_clk)) then
        if (input_clk = last) then
            wait_cnt := wait_cnt + 1;
        else
            wait_cnt := 0;
        end if;
        if (wait_cnt >= 50) then
            debounce_input_clk <= input_clk;
        end if;
        last := input_clk;
    end if;
end process;

process (debounce_input_clk)
variable clk_cnt : integer := 0;
variable reg : unsigned (7 downto 0);
variable data : unsigned (15 downto 0);
begin
    if (rising_edge(debounce_input_clk)) then
        case clk_cnt is
        when 0      => clk_cnt := 1; reg := input;              led_a <= '0'; led_b <= '1';
        when 1      => clk_cnt := 2; data(7 downto 0) := input; led_a <= '1'; led_b <= '0';
        when others => clk_cnt := 0; data(15 downto 8) := input;led_a <= '1'; led_b <= '1';
            case reg is
                when x"00" => stepper_a_pos := signed(data); 
                when x"01" => stepper_b_pos := signed(data); 
                when x"02" => servo_a_width := data; 
                when x"03" => servo_b_width := data;
                when others => led_a <= '0'; led_b <= '0';
            end case;
        end case;
    end if;
end process;
end behav1;
