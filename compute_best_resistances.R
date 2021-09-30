## script to compute ideal resistor values for the biggest range of ladder button configs
library(dplyr)
library(ggplot2)
library(tidyr)
library(viridis)

voltage = 5
r_a_1 = 0
r_a_2 = 165
r_a_3 = 652

r_b_1 = 165
r_b_2 = 652

r_1_values <- seq(from=50, to=1500, by=50)
r_extra_values <- seq(from=0, to=700, by=50)

resistor_values_A <- 
  expand.grid(r_1 = r_1_values, r_extra=r_extra_values) %>%
  as_tibble() %>%
  mutate(
    v_a_1 = voltage * (r_a_1 + r_extra) / (r_1 + r_a_1 + r_extra),
    v_a_2 = voltage * (r_a_2 + r_extra) / (r_1 + r_a_2 + r_extra),
    v_a_3 = voltage * (r_a_3 + r_extra) / (r_1 + r_a_3 + r_extra),
    v_b_1 = voltage * (r_b_1) / (r_1 + r_b_1),
    v_b_2 = voltage * (r_b_2) / (r_1 + r_b_2)
  ) %>%
  pivot_longer(cols = matches("^v_"), names_to = "switch", values_to = "voltages") %>%
  mutate(wire = gsub(pattern = "v_(a|b)_\\d", replacement = "\\1", x = switch))

ggplot(data = resistor_values_A, aes(x = r_1, y = voltages)) +
  geom_line(aes(group = switch, colour = switch, linetype = wire)) +
  facet_grid(~r_extra) +
  theme_minimal() +
  scale_color_viridis(discrete = TRUE) +
  scale_x_continuous(breaks = seq(0,1500, by = 100), minor_breaks = NULL) +
  theme(axis.text.x = element_text(angle = 90, hjust = 1, vjust = 0.5)) +
  labs(
    title = "Voltages produced by switches relative to R1 and R extra on wire A.", 
    subtitle = "Separate panels show values of R extra, which is added in series with wire A.",
    caption = "The best values to choose are ones that produce the most spread-apart voltage on the 5 switch states, in my case I'm going for R1=500ohm and Rextra=300ohm"
    ) +
  NULL

ggsave(filename = "~/arduino_micra_swc/resistance_plot_extra_resistor_A.pdf", dpi = 300, width = 12, height = 8)

resistor_values_B <-
  expand.grid(r_1 = r_1_values, r_extra=r_extra_values) %>%
  as_tibble() %>%
  mutate(
    v_a_1 = voltage * (r_a_1) / (r_1 + r_a_1),
    v_a_2 = voltage * (r_a_2) / (r_1 + r_a_2),
    v_a_3 = voltage * (r_a_3) / (r_1 + r_a_3),
    v_b_1 = voltage * (r_b_1 + r_extra) / (r_1 + r_b_1 + r_extra),
    v_b_2 = voltage * (r_b_2 + r_extra) / (r_1 + r_b_2 + r_extra)
  ) %>%
  pivot_longer(cols = matches("^v_"), names_to = "switch", values_to = "voltages") %>%
  mutate(wire = gsub(pattern = "v_(a|b)_\\d", replacement = "\\1", x = switch))


ggplot(data = resistor_values_B, aes(x = r_1, y = voltages)) +
  geom_line(aes(group = switch, colour = switch, linetype = wire)) +
  facet_grid(~r_extra) +
  theme_minimal() +
  scale_color_viridis(discrete = TRUE) +
  scale_x_continuous(breaks = seq(0,1500, by = 100), minor_breaks = NULL) +
  theme(axis.text.x = element_text(angle = 90, hjust = 1, vjust = 0.5)) +
  labs(
    title = "Voltages produced by switches relative to R1 and R extra on wire B.", 
    subtitle = "Separate panels show values of R extra, which is added in series with wire B.",
    caption = "The best values to choose are ones that produce the most spread-apart voltage on the 5 switch states, in my case I'm going for R1=600ohm and Rextra=300ohm"
  ) +
  NULL

ggsave(filename = "~/arduino_micra_swc/resistance_plot_extra_resistor_B.pdf", dpi = 300, width = 12, height = 8)
