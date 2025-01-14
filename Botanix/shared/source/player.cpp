#pragma once

#include "player.h"

void Player::update(float dt)
{
	pos += v * dt;
}

float Player::get_maxspeed()
{
	return this->SPEED;
}

sf::Vector2f Player::get_pos()
{
	return this->pos;
}

void Player::set_pos(sf::Vector2f new_pos)
{
	this->pos.x = new_pos.x;
	this->pos.y = new_pos.y;
}

sf::Vector2f Player::get_vel()
{
	return this->v;
}

float Player::get_rad()
{
	return this->RADIUS;
}

void Player::set_vel(sf::Vector2f vel)
{
	this->v = vel;
}

void Player::increase_score()
{
	this->score++;
	if (this->score == FINAL_SCORE)
	{
		this->winner = true;
	}
}

int Player::get_score()
{
	return this->score;
}

void Player::set_score(int score)
{
	this->score = score;
}

bool Player::IfWinner()
{
	return this->winner;
}

void Player::won_the_game()
{
	this->winner = true;
}

void Player::no_longer_winner()
{
	this->winner = false;
}

bool Player::IfReady()
{
	return this->ready;
}

sf::Vector2f Player::get_controls()
{
	return this->controls;
}

void Player::set_controls(sf::Vector2f controls)
{
	this->controls = controls;
}

void Player::player_ready()
{
	this->ready = true;
}

void Player::player_not_ready()
{
	this->ready = false;
}

int Player::get_selected_hero()
{
	return this->selected_hero_num;
}

void Player::set_selected_hero(int hero)
{
	this->selected_hero_num = hero;
}

bool Player::isHeroSelected()
{
	return this->is_hero_selected;
}

void Player::setHeroSelectionConfirm(bool x)
{
	this->is_hero_selected = x;
}

std::string Player::getNickname()
{
	return this->nickname;
}

void Player::setNickname(std::string name)
{
	this->nickname = name;
}
